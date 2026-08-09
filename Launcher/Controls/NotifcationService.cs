using System.Collections.ObjectModel;
using System.Threading.Tasks;
using Avalonia.Threading;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.ViewModels;

namespace DaedalusLauncher.Controls;

public partial class NotificationService : ObservableObject
{
    public static ObservableCollection<NotificationViewModel> Notifications { get; } = new();

    public static void Show(string message, string type = "info", int durationMs = 3000)
    {
        var notification = new NotificationViewModel(message, type);
        Notifications.Add(notification);

        // Auto-dismiss after the specified duration
        if (durationMs > 2300)
        {
            _ = Task.Delay(durationMs).ContinueWith(_ =>
                Dispatcher.UIThread.Post(() => Notifications.Remove(notification)));
        }
    }

    [RelayCommand]
    private void Dismiss(NotificationViewModel notification)
    {
        Notifications.Remove(notification);
    }
}