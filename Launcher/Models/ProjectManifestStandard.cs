using System;
using System.Text.Json.Serialization;
using CommunityToolkit.Mvvm.ComponentModel;

namespace DaedalusLauncher.Models;

public class ProjectManifest
{
    [JsonPropertyName("ManifestVersion")]
    public int ManifestVersion { get; set; } = 1;

    [JsonPropertyName("Project")]
    public ProjectIdentity Project { get; set; } = new();

    [JsonPropertyName("Directories")]
    public ProjectDirectories Directories { get; set; } = new();

    [JsonPropertyName("GraphicsDefaults")]
    public ProjectGraphicsDefaults GraphicsDefaults { get; set; } = new();
}

public class ProjectIdentity
{
    [JsonPropertyName("Id")]
    public Guid Id { get; set; } = Guid.NewGuid();

    [JsonPropertyName("Name")]
    public string Name { get; set; } = string.Empty;

    [JsonPropertyName("Author")]
    public string Author { get; set; } = string.Empty;

    [JsonPropertyName("EngineVersion")]
    public string EngineVersion { get; set; } = string.Empty;

    [JsonPropertyName("TargetNetVersion")]
    public string TargetNetVersion { get; set; } = string.Empty;

    [JsonPropertyName("GitInitialised")]
    public bool GitInitialised { get; set; } = false;
}

public class ProjectDirectories
{
    [JsonPropertyName("AssetDirectory")]
    public string AssetDirectory { get; set; } = "Assets";

    [JsonPropertyName("CacheDirectory")]
    public string CacheDirectory { get; set; } = "Cache";

    [JsonPropertyName("StartScene")]
    public string StartScene { get; set; } = string.Empty;
}

public class ProjectGraphicsDefaults
{
    [JsonPropertyName("RenderApiBackend")]
    public string RenderApiBackend { get; set; } = "Vulkan";

    [JsonPropertyName("DefaultWindowMode")]
    public string DefaultWindowMode { get; set; } = "Windowed";

    [JsonPropertyName("TargetResolution")]
    public string TargetResolution { get; set; } = "1920x1080";
}