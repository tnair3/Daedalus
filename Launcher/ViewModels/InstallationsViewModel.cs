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

    [ObservableProperty] private string _status = "Up to date";
    
    private const string RemoteUrl = "https://raw.githubusercontent.com/tnair3/Daedalus/main/versions.json";
    private static string LocalPath => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "release.json");
    private static readonly HttpClient _httpClient = new HttpClient();

    public async Task CheckVersionsAsync()
    {
        Status = "Checking...";
        
        if (!File.Exists(LocalPath))
        {
            var initialManifest = new VersionsManifest
            {
                Launcher = new ModuleInfo { Name = "Launcher", Version = "1.0.0-alpha.1" },
                Editor = new ModuleInfo { Name = "Editor", Version = "1.0.0-alpha.1" },
                Engine = new ModuleInfo { Name = "Engine", Version = "1.0.0-alpha.1" }
            };

            var options = new JsonSerializerOptions { WriteIndented = true };
            string initialJson = JsonSerializer.Serialize(initialManifest, options);
            
            File.WriteAllText(LocalPath, initialJson);
        }

        try
        {
            using var stream = new FileStream(LocalPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            var localData = await JsonSerializer.DeserializeAsync<VersionsManifest>(stream);

            if (localData != null)
            {
                LauncherVersion = localData.Launcher.Version;
                EditorVersion = localData.Editor.Version;
                EngineVersion = localData.Engine.Version;
            }
            
            var remoteJson = await _httpClient.GetStringAsync(RemoteUrl);
            var remoteData = JsonSerializer.Deserialize<VersionsManifest>(remoteJson);

            if (localData != null && remoteData != null)
            {
                IsLauncherUpdateAvailable = (remoteData.Launcher.Version != localData.Launcher.Version);
                IsEditorUpdateAvailable = (remoteData.Editor.Version != localData.Editor.Version);
                IsEngineUpdateAvailable = (remoteData.Engine.Version != localData.Engine.Version);
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
        // Write logic to create a log
    }
}