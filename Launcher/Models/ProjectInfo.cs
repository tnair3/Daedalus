using System;

namespace DaedalusLauncher.Models;

public class ProjectInfo
{
    public bool IsFavourite { get; set; } = false;
    public string Name { get; set; } = string.Empty;
    public string EngineVersion { get; set; } = string.Empty;
    public DateTime LastModified { get; set; } = DateTime.Now;

    public string ProjectPath { get; set; } = string.Empty;
    public string Author { get; set; } = string.Empty;
    public string TargetNetVersion { get; set; } = string.Empty;
    public string RenderApiBackend { get; set; } = string.Empty;
    public string DefaultWindowMode { get; set; } = string.Empty;
    public string TargetResolution { get; set; } = string.Empty;
    public bool GitInitialised { get; set; } = false;
}