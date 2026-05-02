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
    
    private const string RemoteUrl = "https://raw.githubusercontent.com/tnair3/Daedalus/main/versions.json";
    private static string LocalPath => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "release.json");
    private static readonly HttpClient _httpClient = new HttpClient();

    public async Task CheckVersionsAsync()
    {
        if (!File.Exists(LocalPath))
        {
            var initialManifest = new VersionsManifest
            {
                Launcher = new ModuleInfo { Version = "1.0.0-alpha.1" },
                Editor = new ModuleInfo { Version = "1.0.0-alpha.1" },
                Engine = new ModuleInfo { Version = "1.0.0-alpha.1" }
            };

            var options = new JsonSerializerOptions { WriteIndented = true };
            string initialJson = JsonSerializer.Serialize(initialManifest, options);
            
            File.WriteAllText(LocalPath, initialJson);
        }

        try
        {
            LauncherVersion = "Checking...";
            EditorVersion = "Checking...";
            EngineVersion = "Checking...";
            
            using var stream = new FileStream(LocalPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            var localData = await JsonSerializer.DeserializeAsync<VersionsManifest>(stream);

            var remoteJson = await _httpClient.GetStringAsync(RemoteUrl);
            var remoteData = JsonSerializer.Deserialize<VersionsManifest>(remoteJson);

            if (remoteData != null && localData != null)
            {
                LauncherVersion = localData.Launcher.Version;
                EditorVersion = localData.Editor.Version;
                EngineVersion = localData.Engine.Version;
                
                IsLauncherUpdateAvailable = (remoteData.Launcher.Version != localData.Launcher.Version);
                IsEditorUpdateAvailable = (remoteData.Editor.Version != localData.Editor.Version);
                IsEngineUpdateAvailable = (remoteData.Engine.Version != localData.Engine.Version);
            }
        }
        catch (HttpRequestException)
        {
            SetErrorState("Server unreachable");
        }
        catch (Exception)
        {
            SetErrorState("Update check failed");
        }
        // Write logic to create a log
    }

    private void SetErrorState(string message)
    {
        LauncherVersion = EditorVersion = EngineVersion = message;
    }
}