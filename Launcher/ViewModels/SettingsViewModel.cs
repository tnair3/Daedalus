using System;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;

namespace DaedalusLauncher.ViewModels;

public partial class SettingsViewModel : ViewModelBase
{
    [RelayCommand]
    private void Save()
    {
        // Add logic to save settings to a file/database here
        RequestClose(true);
    }

    [RelayCommand]
    private void Cancel()
    {
        RequestClose(false);
    }
    
    public event Action<bool>? CloseRequested;

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }
}