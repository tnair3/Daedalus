using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Avalonia.Controls;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;

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
    public async Task CreateProjectAsync()
    {
        if (string.IsNullOrWhiteSpace(ProjectName) || string.IsNullOrWhiteSpace(ProjectPath))
            return;

        string targetDirectory = Path.Combine(ProjectPath, ProjectName);
        
        await Task.CompletedTask; 
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