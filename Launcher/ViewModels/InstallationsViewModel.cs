using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class InstallationsViewModel : ViewModelBase
{
    private const string RemoteUrl = "https://raw.githubusercontent.com/tnair3/Daedalus/main/Maintenance/versions.json";
    private static readonly HttpClient HttpClient = new();
    private static string LocalPath => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "release.json");
    
    [ObservableProperty] private string _launcherVersion = "Loading...";
    [ObservableProperty] private UpdateStatus _launcherUpdateStatus;
    [ObservableProperty] private string _editorVersion = "Loading...";
    [ObservableProperty] private UpdateStatus _editorUpdateStatus;
    [ObservableProperty] private string _engineVersion = "Loading...";
    [ObservableProperty] private UpdateStatus _engineUpdateStatus;
    [ObservableProperty] private string _booksVersion = "Loading...";
    [ObservableProperty] private UpdateStatus _booksUpdateStatus;
    [ObservableProperty] private string _status = "Loading...";
    [ObservableProperty] private bool _serverReached = true;
    
    [ObservableProperty] [NotifyPropertyChangedFor(nameof(SelectedModuleName))] private ModuleType _selectedModule;
    [ObservableProperty] private string _selectedModuleVersion;
    [ObservableProperty] private UpdateStatus _selectedModuleUpdateStatus;
    [ObservableProperty] private string _selectedModuleDescription;
    [ObservableProperty] private ObservableCollection<string> _selectedModuleChangelog =
    [
        "Placeholder changelog text",
        "Placeholder changelog text",
        "Placeholder changelog text"
    ];

    public string SelectedModuleName => SelectedModule switch
    {
        ModuleType.Launcher => "Daedalus Launcher",
        ModuleType.Editor   => "Project Editor",
        ModuleType.Engine   => "Daedalus Engine",
        ModuleType.Books    => "Application Documentation",
        _                   => "Unknown Module"
    };

    public InstallationsViewModel()
    {
        _ = CheckVersionsAsync();

        SelectedModule = ModuleType.Launcher;
        SelectedModuleVersion = LauncherVersion;
        SelectedModuleUpdateStatus = UpdateStatus.Unknown;
        SelectedModuleDescription = "Unknown Description";
    }

    public async Task CheckVersionsAsync()
    {
        Status = "Checking...";

        if (!File.Exists(LocalPath))
        {
            var root = new ManifestRoot
            {
                Modules = new VersionsManifest
                {
                    Launcher = new ModuleInfo { Name = "Launcher", Version = "1.0.0-alpha.1", InstallPath = "../Launcher" },
                    Editor   = new ModuleInfo { Name = "Editor", Version = "1.0.0-alpha.1", InstallPath = "../Editor" },
                    Engine   = new ModuleInfo { Name = "Engine", Version = "1.0.0-alpha.1", InstallPath = "../Engine" },
                    Books    = new ModuleInfo { Name = "Books", Version = "0000-00-00", InstallPath = "../Books" }
                }
            };

            var options = new JsonSerializerOptions { WriteIndented = true };
            string initialJson = JsonSerializer.Serialize(root, options);
            await File.WriteAllTextAsync(LocalPath, initialJson);
        }

        try
        {
            var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };

            await using var localStream = new FileStream(LocalPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            var localRoot = JsonSerializer.Deserialize<ManifestRoot>(localStream, options);
            var localData = localRoot?.Modules;

            if (localData != null)
            {
                LauncherVersion = localData.Launcher.Version;
                EditorVersion = localData.Editor.Version;
                EngineVersion = localData.Engine.Version;
                BooksVersion = localData.Books.Version;

                var remoteJson = await HttpClient.GetStringAsync(RemoteUrl);
                var remoteRoot = JsonSerializer.Deserialize<ManifestRoot>(remoteJson, options);
                var remoteData = remoteRoot?.Modules;

                if (remoteData != null)
                {
                    LauncherUpdateStatus = GetUpdateStatus(localData.Launcher.Version, remoteData.Launcher.Version);
                    EditorUpdateStatus = GetUpdateStatus(localData.Editor.Version, remoteData.Editor.Version);
                    EngineUpdateStatus = GetUpdateStatus(localData.Engine.Version, remoteData.Engine.Version);
                    BooksUpdateStatus = GetUpdateStatus(localData.Books.Version, remoteData.Books.Version);
                }

                Status = (LauncherUpdateStatus == UpdateStatus.UpdateAvailable ||
                          EditorUpdateStatus == UpdateStatus.UpdateAvailable ||
                          EngineUpdateStatus == UpdateStatus.UpdateAvailable ||
                          BooksUpdateStatus == UpdateStatus.UpdateAvailable)
                    ? "Updates available"
                    : "Up to date";
            }

            ServerReached = true;
        }
        catch (Exception)
        {
            Status = "Server unreachable";
            ServerReached = false;
            SetAllModuleStatuses(UpdateStatus.ServerUnreachable);
        }
        finally
        {
            UpdateSelectedModuleDetails(SelectedModule);
        }
    }

    [RelayCommand]
    private async Task UpdateSelectedModuleAsync()
    {
        await Task.CompletedTask;
    }

    [RelayCommand]
    private void UpdateSelectedModuleDetails(ModuleType moduleType)
    {
        SelectedModule = moduleType;
        switch (SelectedModule)
        {
            case ModuleType.Launcher:
                SelectedModuleVersion = LauncherVersion;
                SelectedModuleUpdateStatus = LauncherUpdateStatus;
                break;
            case ModuleType.Editor:
                SelectedModuleVersion = EditorVersion;
                SelectedModuleUpdateStatus = EditorUpdateStatus;
                break;
            case ModuleType.Engine:
                SelectedModuleVersion = EngineVersion;
                SelectedModuleUpdateStatus = EngineUpdateStatus;
                break;
            case ModuleType.Books:
                SelectedModuleVersion = BooksVersion;
                SelectedModuleUpdateStatus = BooksUpdateStatus;
                break;
        }
    }

    private void SetAllModuleStatuses(UpdateStatus status)
    {
        LauncherUpdateStatus = status;
        EditorUpdateStatus = status;
        EngineUpdateStatus = status;
        BooksUpdateStatus = status;
    }

    private static UpdateStatus GetUpdateStatus(string localVersion, string remoteVersion)
    {
        return remoteVersion != localVersion ? UpdateStatus.UpdateAvailable : UpdateStatus.UpToDate;
    }
}