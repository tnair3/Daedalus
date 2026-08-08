using System;
using Avalonia.Controls;
using CommunityToolkit.Mvvm.Input;

namespace DaedalusLauncher.ViewModels;

public partial class ReportViewModel : ViewModelBase
{
    public event Action<bool>? CloseRequested;

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }
    
    [RelayCommand]
    public void CloseWindow(Window? window)
    {
        if (window != null)
        {
            window.Close();
        }
    }
}