namespace DaedalusLauncher.Models;

public class ModuleInfo
{
    public string Name { get; set; } = string.Empty;
    public string Version { get; set; } = string.Empty;
    public string InstallPath { get; set; } = string.Empty; // Relative path from the .metadata folder to the module root (e.g., "../Engine/")
    public string? RequiredEngine { get; set; } // Optional: Only used if a specific module has a dependency
}