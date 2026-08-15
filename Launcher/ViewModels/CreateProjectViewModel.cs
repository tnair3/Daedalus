using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using CommunityToolkit.Mvvm.Messaging;
using DaedalusLauncher.Controls;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class CreateProjectViewModel : ViewModelBase
{
    public event Action<bool>? CloseRequested;
    
    [ObservableProperty] private string _projectName;
    [ObservableProperty] private string _projectPath;
    [ObservableProperty] private string _projectAuthor;
    [ObservableProperty] private bool _initializeGit;
    [ObservableProperty] private string _selectedTemplate;
    [ObservableProperty] private string _selectedDotNetVersion;
    [ObservableProperty] private string _selectedRenderApi;
    [ObservableProperty] private string _selectedResolution;
    [ObservableProperty] [NotifyPropertyChangedFor(nameof(IsResolutionEditable))] private string _selectedWindowMode;

    public List<string> AvailableTemplates { get; } = ["Minimal Console Core", "2D Blank Canvas", "3D Blank Canvas"];
    public List<string> AvailableDotNetVersions { get; } = [".NET 8.0", ".NET 9.0"];
    public List<string> AvailableRenderApis { get; } = ["Vulkan (Recommended)", "DirectX 12", "OpenGL 4.6"];
    public List<string> AvailableWindowModes { get; } = ["Windowed", "Borderless Fullscreen", "Exclusive Fullscreen"];
    public List<string> AvailableResolutions { get; } = ["1920x1080", "2560x1440", "3840x2160", "1280x720"];

    public bool IsResolutionEditable => SelectedWindowMode == "Windowed";

    public CreateProjectViewModel()
    {
        _projectName = string.Empty;
        _projectPath = string.Empty;
        _projectAuthor = string.Empty;
        _initializeGit = true;
        _selectedTemplate = AvailableTemplates[0];
        _selectedDotNetVersion = AvailableDotNetVersions[0];
        _selectedRenderApi = AvailableRenderApis[0];
        _selectedWindowMode = AvailableWindowModes[0];
        _selectedResolution = AvailableResolutions[0];
    }


    [RelayCommand]
    private async Task CreateProjectAsync()
    {
        if (string.IsNullOrWhiteSpace(ProjectName) || string.IsNullOrWhiteSpace(ProjectPath))
        {
            NotificationService.Show("Project name and path cannot be empty", "error");
            return;
        }

        if (!Directory.Exists(ProjectPath))
        {
            Debug.WriteLine($"Directory does not exist: {ProjectPath}");
            NotificationService.Show($"Directory '{ProjectPath}' does not exist", "error");
            return;
        }

        try
        {
            // Create project directory structure
            string sanitizedProjectName = Regex.Replace(ProjectName.Trim(), @"\s+", "_");
            string fullProjectPath = Path.Combine(ProjectPath, sanitizedProjectName);

            if (Directory.Exists(fullProjectPath))
            {
                Debug.WriteLine($"Error: A directory already exists at '{fullProjectPath}'");
                NotificationService.Show($"A directory already exists at '{fullProjectPath}'", "error");
                return;
            }

            Directory.CreateDirectory(fullProjectPath);
            Directory.CreateDirectory(Path.Combine(fullProjectPath, "Assets"));
            Directory.CreateDirectory(Path.Combine(fullProjectPath, "Cache"));

            string projectFilePath = Path.Combine(fullProjectPath, $"{sanitizedProjectName}.myproject");

            Guid projectId = Guid.NewGuid();
            string engineVersion = Assembly.GetExecutingAssembly()
                .GetCustomAttribute<AssemblyInformationalVersionAttribute>()?
                .InformationalVersion ?? "Unknown";

            var newProjectManifest = new ProjectManifest
            {
                Project = new ProjectIdentity
                {
                    Id = projectId,
                    Name = ProjectName,
                    Author = ProjectAuthor,
                    EngineVersion = engineVersion,
                    TargetNetVersion = SelectedDotNetVersion,
                    GitInitialised = InitializeGit
                },
                Directories = new ProjectDirectories
                {
                    AssetDirectory = "Assets",
                    CacheDirectory = "Cache",
                    StartScene = "Assets/Scenes/Main.scene"
                },
                GraphicsDefaults = new ProjectGraphicsDefaults
                {
                    RenderApiBackend = SelectedRenderApi,
                    DefaultWindowMode = SelectedWindowMode,
                    TargetResolution = SelectedResolution
                }
            };

            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                PropertyNameCaseInsensitive = true
            };

            string jsonContent = JsonSerializer.Serialize(newProjectManifest, options);
            await File.WriteAllTextAsync(projectFilePath, jsonContent);
            
            string localPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "projects.json");

            var newProject = new ProjectInfo
            {
                Id = projectId,
                Name = ProjectName,
                EngineVersion = engineVersion,
                LastModified = DateTime.Now,
                ProjectPath = ProjectPath,
                Author = ProjectAuthor,
                IsFavourite = false,
                TargetNetVersion = SelectedDotNetVersion,
                RenderApiBackend = SelectedRenderApi,
                DefaultWindowMode = SelectedWindowMode,
                TargetResolution = SelectedResolution,
                GitInitialised = InitializeGit
            };

            ProjectRoot root;
            if (File.Exists(localPath))
            {
                string currentJson = await File.ReadAllTextAsync(localPath);
                root = JsonSerializer.Deserialize<ProjectRoot>(currentJson, options) ?? new ProjectRoot();
            }
            else
            {
                root = new ProjectRoot();
            }

            root.Projects.Add(newProject);

            string updatedJson = JsonSerializer.Serialize(root, options);
            await File.WriteAllTextAsync(localPath, updatedJson);

            WeakReferenceMessenger.Default.Send(new NewProjectMessage());
            RequestClose(true);
        }
        catch (Exception ex)
        {
            Debug.WriteLine($"Failed to write project to JSON: {ex.Message}");
            NotificationService.Show("Failed to write project to JSON, check error logs", "error");

            await LogExceptionAsync(ex);
        }
    }

    [RelayCommand]
    private void GoBack()
    {
        RequestClose(false);
    }

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }

    private static async Task LogExceptionAsync(Exception ex)
    {
        string logsDir = Path.Combine(AppContext.BaseDirectory, "logs");

        if (!Directory.Exists(logsDir))
        {
            Directory.CreateDirectory(logsDir);
        }

        try
        {
            string fileName = $"log_{DateTime.Now:yyyy-MM-dd}.txt";
            string filePath = Path.Combine(logsDir, fileName);

            string logEntry = $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss}] EXCEPTION THROWN:" +
                              $"{Environment.NewLine}{ex}" +
                              $"{Environment.NewLine}{new string('-', 80)}{Environment.NewLine}";

            await File.AppendAllTextAsync(filePath, logEntry);
        }
        catch (Exception logEx)
        {
            Debug.WriteLine($"Failed to write to log file: {logEx.Message}");
        }
    }
}