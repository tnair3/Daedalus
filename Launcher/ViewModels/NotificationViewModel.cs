using CommunityToolkit.Mvvm.ComponentModel;

namespace DaedalusLauncher.ViewModels;

public partial class NotificationViewModel(string message, string type = "info") : ObservableObject
{
    public string Message { get; } = message;
    public string Type { get; } = type; // "info", "success", "error", "warning"
}