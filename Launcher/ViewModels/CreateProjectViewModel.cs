using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Threading.Tasks;
using System.Reflection;
using Avalonia.Controls;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using CommunityToolkit.Mvvm.Messaging;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class CreateProjectViewModel : ViewModelBase
{
    public event Action<bool>? CloseRequested;

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }
    
    [ObservableProperty] private string _projectName = string.Empty;
    [ObservableProperty] private string _projectPath = string.Empty;
    [ObservableProperty] private string _projectAuthor = string.Empty;
    
    [ObservableProperty] private string _selectedTemplate;
    [ObservableProperty] private string _selectedDotNetVersion;
    public List<string> AvailableTemplates { get; } = new() { "Minimal Console Core", "2D Blank Canvas", "3D Blank Canvas" };
    public List<string> AvailableDotNetVersions { get; } = new() { ".NET 8.0", ".NET 9.0" };
    
    [ObservableProperty] private string _selectedRenderApi;
    [ObservableProperty] private string _selectedResolution;

    [ObservableProperty]
    [NotifyPropertyChangedFor(nameof(IsResolutionEditable))]
    private string _selectedWindowMode;

    public List<string> AvailableRenderApis { get; } = new() { "Vulkan (Recommended)", "DirectX 12", "OpenGL 4.6" };
    public List<string> AvailableWindowModes { get; } = new() { "Windowed", "Borderless Fullscreen", "Exclusive Fullscreen" };
    public List<string> AvailableResolutions { get; } = new() { "1920x1080", "2560x1440", "3840x2160", "1280x720" };
    
    public bool IsResolutionEditable => SelectedWindowMode == "Windowed";
    
    [ObservableProperty] private bool _initializeGit = true;

    public CreateProjectViewModel()
    {
        _selectedTemplate = AvailableTemplates[0];
        _selectedDotNetVersion = AvailableDotNetVersions[0];
        _selectedRenderApi = AvailableRenderApis[0];
        _selectedWindowMode = AvailableWindowModes[0];
        _selectedResolution = AvailableResolutions[0];
    }

    [RelayCommand]
    public async Task CreateProjectAsync(Window? window)
    {
        if (string.IsNullOrWhiteSpace(ProjectName) || string.IsNullOrWhiteSpace(ProjectPath))
        {
            // TODO: Add visual error output for the user
            return;
        }
        
        if (!Directory.Exists(ProjectPath))
        {
            System.Diagnostics.Debug.WriteLine($"Directory does not exist: {ProjectPath}");
            // TODO: Add visual error output for the user
            return;
        }

        try
        {
            // Create project directory and .myproject file
            string sanitizedProjectName = System.Text.RegularExpressions.Regex.Replace(ProjectName.Trim(), @"\s+", "_");
            string fullProjectPath = Path.Combine(ProjectPath, sanitizedProjectName);
            
            if (Directory.Exists(fullProjectPath))
            {
                System.Diagnostics.Debug.WriteLine($"Error: A directory already exists at '{fullProjectPath}'");
                // TODO: Add visual error output for the user
                return;
            }
            
            Directory.CreateDirectory(fullProjectPath);
            Directory.CreateDirectory(Path.Combine(fullProjectPath, $"Assets"));
            Directory.CreateDirectory(Path.Combine(fullProjectPath, $"Cache"));
            
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
            File.WriteAllText(projectFilePath, jsonContent);
            
            // Update launcher projects.json
            string localPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "projects.json"); // Change to ../.metadata/ as Directory for release build

            ProjectInfo newProject = new ProjectInfo()
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
            
            CloseWindow(window);
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to write project to JSON: {ex.Message}");
            // TODO: Add visual error output for the user
        }
    }

    [RelayCommand]
    public void CloseWindow(Window? window)
    {
        if (window != null)
        {
            window.Close();
        }
    }
}