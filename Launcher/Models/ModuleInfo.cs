namespace DaedalusLauncher.Models;

public class ModuleInfo
{
    public string Name { get; set; } = string.Empty;
    public string Version { get; set; } = string.Empty;
    public string? DownloadUrl { get; set; }
    public string? RequiredEngine { get; set; }
}