using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Platform.Storage;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using CommunityToolkit.Mvvm.Messaging;
using DaedalusLauncher.Controls;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class ProjectsViewModel : ViewModelBase
{
    private static string LocalPath => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "projects.json");
    
    [ObservableProperty] private ObservableCollection<ProjectInfo> _projects = new();
    [ObservableProperty] private ObservableCollection<ProjectInfo> _filteredProjects = new();
    [ObservableProperty] private string _searchQuery = string.Empty;
    
    [ObservableProperty]
    [NotifyPropertyChangedFor(nameof(IsSortedByName))]
    [NotifyPropertyChangedFor(nameof(IsSortedByEngine))]
    [NotifyPropertyChangedFor(nameof(IsSortedByModified))]
    private ProjectsSortBy _sortBy = ProjectsSortBy.LastModified;
    private bool _sortAscending;

    public bool IsSortedByName => SortBy == ProjectsSortBy.Name;
    public bool IsSortedByEngine => SortBy == ProjectsSortBy.Engine;
    public bool IsSortedByModified => SortBy == ProjectsSortBy.LastModified;
    
    [ObservableProperty] private bool _isCreatingProject;
    [ObservableProperty] private CreateProjectViewModel? _createProjectViewModel;


    public ProjectsViewModel()
    {
        WeakReferenceMessenger.Default.Register<NewProjectMessage>(this, async void (r, m) =>
        {
            try
            {
                await LoadProjects();
            }
            catch
            {
                // TODO: Handle exception
            }
        });
    }
    
    public async Task LoadProjects()
    {
        try
        {
            if (!File.Exists(LocalPath))
            {
                EnsureDirectoryExists(LocalPath);

                var newProjectRoot = new ProjectRoot { Projects = new List<ProjectInfo>() };
                var options = new JsonSerializerOptions { WriteIndented = true };
                string initialJson = JsonSerializer.Serialize(newProjectRoot, options);

                await File.WriteAllTextAsync(LocalPath, initialJson);
            }

            string jsonContent = await File.ReadAllTextAsync(LocalPath);
            var root = JsonSerializer.Deserialize<ProjectRoot>(jsonContent);

            if (root?.Projects != null)
            {
                Projects = new ObservableCollection<ProjectInfo>(root.Projects);
                UpdateProjectList(Projects);
            }
        }
        catch (Exception ex)
        {
            Debug.WriteLine($"Failed to initialize project file: {ex.Message}");
        }
    }

    [RelayCommand]
    private async Task LoadExistingProject(Window? window)
    {
        if (window == null) return;

        var topLevel = TopLevel.GetTopLevel(window);
        if (topLevel == null) return;

        var files = await topLevel.StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions
        {
            Title = "Open Existing Project",
            AllowMultiple = false,
            FileTypeFilter =
            [
                new FilePickerFileType("MyProject Files (*.myproject)")
                {
                    Patterns = ["*.myproject"]
                }
            ]
        });

        if (files.Count == 0) return;

        string filePath = files[0].Path.LocalPath;
        if (!File.Exists(filePath)) return;

        try
        {
            string jsonContent = await File.ReadAllTextAsync(filePath);
            var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
            var manifest = JsonSerializer.Deserialize<ProjectManifest>(jsonContent, options);

            if (manifest?.Project == null) return;

            string? projectDirectory = Path.GetDirectoryName(filePath);

            if (Projects.Any(p => p.Id == manifest.Project.Id || p.ProjectPath == projectDirectory))
            {
                Debug.WriteLine("Project already exists in launcher.");
                NotificationService.Show("Project already exists in launcher.", "error");
                return;
            }

            var newProject = new ProjectInfo
            {
                Id = manifest.Project.Id,
                Name = manifest.Project.Name,
                Author = manifest.Project.Author,
                EngineVersion = manifest.Project.EngineVersion,
                TargetNetVersion = manifest.Project.TargetNetVersion,
                GitInitialised = manifest.Project.GitInitialised,
                RenderApiBackend = manifest.GraphicsDefaults.RenderApiBackend,
                DefaultWindowMode = manifest.GraphicsDefaults.DefaultWindowMode,
                TargetResolution = manifest.GraphicsDefaults.TargetResolution,
                ProjectPath = projectDirectory ?? filePath,
                LastModified = File.GetLastWriteTime(filePath),
                IsFavourite = false
            };

            Projects.Add(newProject);
            UpdateProjectList(Projects);
            await SaveProjectsToDisk();
        }
        catch (Exception ex)
        {
            Debug.WriteLine($"Failed to load existing project: {ex.Message}");
        }
    }

    [RelayCommand]
    private void SortItems(ProjectsSortBy sortBy)
    {
        if (sortBy == SortBy)
        {
            _sortAscending = !_sortAscending;
        }
        else if (sortBy == ProjectsSortBy.LastModified)
        {
            _sortAscending = false;
        }
        else
        {
            _sortAscending = true;
        }

        SortBy = sortBy;
        UpdateProjectList(Projects);
    }

    [RelayCommand]
    private async Task SetFavourite(ProjectInfo project)
    {
        try
        {
            project.IsFavourite = !project.IsFavourite;
            UpdateProjectList(Projects);
            await SaveProjectsToDisk();
        }
        catch (Exception ex)
        {
            Debug.WriteLine($"Failed to update favorite or save JSON: {ex.Message}");
        }
    }

    [RelayCommand]
    private void OpenCreateProject()
    {
        var createProjectVm = new CreateProjectViewModel();
        createProjectVm.CloseRequested += OnCreateProjectClosed;

        CreateProjectViewModel = createProjectVm;
        IsCreatingProject = true;
    }
    
    partial void OnSearchQueryChanged(string value)
    {
        UpdateProjectList(Projects);
    }

    private void UpdateProjectList(IEnumerable<ProjectInfo> sourceItems)
    {
        if (!string.IsNullOrWhiteSpace(SearchQuery))
        {
            sourceItems = sourceItems.Where(p =>
                p.Name.Contains(SearchQuery, StringComparison.OrdinalIgnoreCase));
        }

        var baseQuery = sourceItems.OrderByDescending(p => p.IsFavourite);
        IOrderedEnumerable<ProjectInfo> orderedQuery = SortBy switch
        {
            ProjectsSortBy.Name => _sortAscending
                ? baseQuery.ThenBy(p => p.Name)
                : baseQuery.ThenByDescending(p => p.Name),

            ProjectsSortBy.Engine => _sortAscending
                ? baseQuery.ThenBy(p => p.EngineVersion)
                : baseQuery.ThenByDescending(p => p.EngineVersion),

            ProjectsSortBy.LastModified => _sortAscending
                ? baseQuery.ThenBy(p => p.LastModified)
                : baseQuery.ThenByDescending(p => p.LastModified),

            _ => baseQuery.ThenByDescending(p => p.LastModified)
        };

        FilteredProjects = new ObservableCollection<ProjectInfo>(orderedQuery.ToList());
    }

    private async Task SaveProjectsToDisk()
    {
        var projectRoot = new ProjectRoot { Projects = Projects.ToList() };
        var options = new JsonSerializerOptions { WriteIndented = true };
        string json = JsonSerializer.Serialize(projectRoot, options);

        await File.WriteAllTextAsync(LocalPath, json);
    }

    private async void OnCreateProjectClosed(bool wasCreated)
    {
        if (CreateProjectViewModel != null)
        {
            CreateProjectViewModel.CloseRequested -= OnCreateProjectClosed;
        }

        IsCreatingProject = false;
        CreateProjectViewModel = null;

        if (wasCreated)
        {
            await LoadProjects();
        }
    }

    private static void EnsureDirectoryExists(string path)
    {
        string directory = Path.GetDirectoryName(path) ?? string.Empty;
        if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
        {
            Directory.CreateDirectory(directory);
        }
    }
}