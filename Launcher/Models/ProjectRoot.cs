using System.Collections.Generic;

namespace DaedalusLauncher.Models;

public class ProjectRoot
{
    public List<ProjectInfo> Projects { get; set; } = new List<ProjectInfo>();
}