using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace DaedalusLauncher.Models;

public class ProjectRoot
{
    [JsonPropertyName("projects")] public List<ProjectInfo> Projects { get; set; } = new List<ProjectInfo>();
}

public class ProjectInfo
{
    [JsonPropertyName("id")] public Guid Id { get; set; } = Guid.NewGuid();
    public string Name { get; set; } = string.Empty;
    public string EngineVersion { get; set; } = string.Empty;
    public DateTime LastModified { get; set; } = DateTime.Now;

    public string ProjectPath { get; set; } = string.Empty;
    public string Author { get; set; } = string.Empty;
    public bool IsFavourite { get; set; } = false;
    public string TargetNetVersion { get; set; } = string.Empty;
    public string RenderApiBackend { get; set; } = string.Empty;
    public string DefaultWindowMode { get; set; } = string.Empty;
    public string TargetResolution { get; set; } = string.Empty;
    public bool GitInitialised { get; set; } = false;
}