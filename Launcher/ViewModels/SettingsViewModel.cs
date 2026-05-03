using System;
using System.Collections.Generic;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class SettingsViewModel : ViewModelBase
{
    public IEnumerable<SettingsCategory> Categories { get; } = Enum.GetValues<SettingsCategory>();
    [ObservableProperty] private SettingsCategory _selectedCategory;
    
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