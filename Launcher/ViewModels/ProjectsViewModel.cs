using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Reactive.Linq;
using System.Text.Json;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using CommunityToolkit.Mvvm.Messaging;
using DaedalusLauncher.Models;
using ReactiveUI;

namespace DaedalusLauncher.ViewModels;

public partial class ProjectsViewModel : ViewModelBase
{
    private static string LocalPath => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "projects.json"); // Change to ../.metadata/ as Directory for release build
    
    [ObservableProperty] private ObservableCollection<ProjectInfo> _projects = new();
    
    public Interaction<CreateProjectViewModel, bool> ShowCreateProjectDialog { get; } = new();

    public ProjectsViewModel()
    {
        WeakReferenceMessenger.Default.Register<ProjectCreatedMessage>(this, async (r, m) =>
        {
            await LoadProjects();
        });
    }
    
    public async Task LoadProjects()
    {
        try
        {
            if (!File.Exists(LocalPath))
            {
                string directory = Path.GetDirectoryName(LocalPath) ?? string.Empty;
                if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                }
                
                var newProjectRoot = new ProjectRoot
                {
                    Projects = new List<ProjectInfo>() 
                };

                var options = new JsonSerializerOptions { WriteIndented = true };
                string initialJson = JsonSerializer.Serialize(newProjectRoot, options);
                
                await File.WriteAllTextAsync(LocalPath, initialJson);
            }
            
            string jsonContent = await File.ReadAllTextAsync(LocalPath);
            var root = JsonSerializer.Deserialize<ProjectRoot>(jsonContent);
            
            if (root?.Projects != null)
            {
                Projects.Clear();
                foreach (var project in root.Projects)
                {
                    Projects.Add(project);
                }
            }
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to initialize project file: {ex.Message}");
        }
    }
    
    [RelayCommand]
    public async Task OpenCreateProjectWindow()
    {
        var createProjectVm = new CreateProjectViewModel();
        
        bool isCreated = await ShowCreateProjectDialog.Handle(createProjectVm);
        
        if (isCreated)
        {
            // Reload from disk or append the new project directly to the Projects collection
            await LoadProjects();
        }
    }
}