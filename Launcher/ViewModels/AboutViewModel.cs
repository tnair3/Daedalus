using System;
using Avalonia.Controls;
using CommunityToolkit.Mvvm.Input;

namespace DaedalusLauncher.ViewModels;

public partial class AboutViewModel : ViewModelBase
{
    public event Action<bool>? CloseRequested;

    [RelayCommand]
    private void CloseWindow(Window? window)
    {
        if (window != null)
        {
            window.Close();
        }
    }

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }
}