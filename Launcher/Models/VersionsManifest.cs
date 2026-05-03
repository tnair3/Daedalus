using System.Text.Json.Serialization;

namespace DaedalusLauncher.Models;

public class ManifestRoot
{
    [JsonPropertyName("modules")] public VersionsManifest Modules { get; set; } = new();
    [JsonPropertyName("last_updated")] public string? LastUpdated { get; set; }
}

public class VersionsManifest
{
    [JsonPropertyName("launcher")] public ModuleInfo Launcher { get; set; } = new();
    [JsonPropertyName("editor")] public ModuleInfo Editor { get; set; } = new();
    [JsonPropertyName("engine")] public ModuleInfo Engine { get; set; } = new();
    [JsonPropertyName("books")] public ModuleInfo Books { get; set; } = new();
}