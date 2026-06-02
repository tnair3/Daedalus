using System;
using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using System.IO;
using CommunityToolkit.Mvvm.ComponentModel;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class InstallationsViewModel : ViewModelBase
{
    public InstallationsViewModel()
    {
        _ = CheckVersionsAsync();
    }
    
    [ObservableProperty] private string _launcherVersion = "Loading...";
    [ObservableProperty] private bool _isLauncherUpdateAvailable;

    [ObservableProperty] private string _editorVersion = "Loading...";
    [ObservableProperty] private bool _isEditorUpdateAvailable;

    [ObservableProperty] private string _engineVersion = "Loading...";
    [ObservableProperty] private bool _isEngineUpdateAvailable;
    
    [ObservableProperty] private string _booksVersion = "Loading...";
    [ObservableProperty] private bool _isBooksUpdateAvailable;

    [ObservableProperty] private string _status = "Loading...";
    
    private const string RemoteUrl = "https://raw.githubusercontent.com/tnair3/Daedalus/main/Maintenance/versions.json";
    private static string LocalPath => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "release.json"); // Change to ../.metadata/ as Directory for release build
    private static readonly HttpClient _httpClient = new HttpClient();

    public async Task CheckVersionsAsync()
    {
        Status = "Checking...";
        
        if (!File.Exists(LocalPath))
        {
            var root = new ManifestRoot
            {
                Modules = new VersionsManifest
                {
                    Launcher = new ModuleInfo { Name = "Launcher", Version = "1.0.0-alpha.1", InstallPath = "../Launcher"},
                    Editor = new ModuleInfo { Name = "Editor", Version = "1.0.0-alpha.1", InstallPath = "../Editor" },
                    Engine = new ModuleInfo { Name = "Engine", Version = "1.0.0-alpha.1", InstallPath = "../Engine" },
                    Books = new ModuleInfo { Name = "Books", Version = "0000-00-00", InstallPath = "../Books" }
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

                var remoteJson = await _httpClient.GetStringAsync(RemoteUrl);
                var remoteRoot = JsonSerializer.Deserialize<ManifestRoot>(remoteJson, options);
                var remoteData = remoteRoot?.Modules;
                
                if (remoteData != null)
                {
                    // Update to proper SemVer/Date-Based version checking
                    IsLauncherUpdateAvailable = (remoteData.Launcher.Version != localData.Launcher.Version);
                    IsEditorUpdateAvailable = (remoteData.Editor.Version != localData.Editor.Version);
                    IsEngineUpdateAvailable = (remoteData.Engine.Version != localData.Engine.Version);
                    IsBooksUpdateAvailable = (remoteData.Books.Version != localData.Books.Version);
                }
            }
            
            Status = "Up to date";
        }
        catch (HttpRequestException)
        {
            Status = "Server unreachable";
        }
        catch (Exception)
        {
            Status = "Update check failed";
        }
        // Write logic to create a log if there is an error
    }
}