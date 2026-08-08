using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
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
    private static string LocalPath => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "projects.json");
    
    [ObservableProperty] private ObservableCollection<ProjectInfo> _projects = new();
    [ObservableProperty] private string _searchQuery = string.Empty;
    [ObservableProperty] private ObservableCollection<ProjectInfo> _filteredProjects = new();
    
    [ObservableProperty]
    [NotifyPropertyChangedFor(nameof(IsSortedByName))]
    [NotifyPropertyChangedFor(nameof(IsSortedByEngine))]
    [NotifyPropertyChangedFor(nameof(IsSortedByModified))]
    private ProjectsSortBy _sortBy = ProjectsSortBy.LastModified;
    private bool _sortAscending = false;
    public bool IsSortedByName => SortBy ==  ProjectsSortBy.Name;
    public bool IsSortedByEngine => SortBy ==  ProjectsSortBy.Engine;
    public bool IsSortedByModified => SortBy ==   ProjectsSortBy.LastModified;
    
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
                var incomingList = root.Projects;
                Projects = new ObservableCollection<ProjectInfo>(incomingList);
                
                UpdateProjectList(Projects);
            }
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to initialize project file: {ex.Message}");
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
            System.Diagnostics.Debug.WriteLine($"Failed to update favorite or save JSON: {ex.Message}");
        }
    }
    
    private async Task SaveProjectsToDisk()
    {
        var projectRoot = new ProjectRoot { Projects = Projects.ToList() };
        var options = new JsonSerializerOptions { WriteIndented = true };
        string json = JsonSerializer.Serialize(projectRoot, options);
    
        await File.WriteAllTextAsync(LocalPath, json);
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

    [RelayCommand]
    private async Task OpenCreateProjectWindow()
    {
        var createProjectVm = new CreateProjectViewModel();
        bool isCreated = await ShowCreateProjectDialog.Handle(createProjectVm);
        
        if (isCreated)
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