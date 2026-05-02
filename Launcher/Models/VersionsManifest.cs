namespace DaedalusLauncher.Models;

public class VersionsManifest
{
    public ModuleInfo Launcher { get; set; } = new();
    public ModuleInfo Editor { get; set; } = new();
    public ModuleInfo Engine { get; set; } = new();
}