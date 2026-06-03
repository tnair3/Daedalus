using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Threading.Tasks;
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
        if (string.IsNullOrWhiteSpace(ProjectName) || string.IsNullOrWhiteSpace(ProjectPath) ||
            string.IsNullOrWhiteSpace(ProjectAuthor))
        {
            return;
            // Create visual output
        }

        try
        {
            string localPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "projects.json"); // Change to ../.metadata/ as Directory for release build

            ProjectInfo newProject = new ProjectInfo()
            {
                Id = Guid.NewGuid(),
                Name = ProjectName,
                EngineVersion = "v1.0.0-beta.1", // update to dynamically change based on installed engine version
                LastModified = DateTime.Now,
                ProjectPath = this.ProjectPath,
                Author = ProjectAuthor,
                IsFavourite = false,
                TargetNetVersion = SelectedDotNetVersion,
                RenderApiBackend = SelectedRenderApi,
                DefaultWindowMode = SelectedWindowMode,
                TargetResolution = SelectedResolution,
                GitInitialised = InitializeGit
            };
            
            var options = new JsonSerializerOptions 
            { 
                WriteIndented = true,
                PropertyNameCaseInsensitive = true
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
            
            WeakReferenceMessenger.Default.Send(new ProjectCreatedMessage());
            
            CloseWindow(window);
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to write project to JSON: {ex.Message}");
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