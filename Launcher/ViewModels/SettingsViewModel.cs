using System;
using System.Collections.Generic;
using CommunityToolkit.Mvvm.ComponentModel;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class SettingsViewModel : ViewModelBase
{
    public IEnumerable<SettingsCategory> Categories { get; } = Enum.GetValues<SettingsCategory>();
    [ObservableProperty] private SettingsCategory _selectedCategory;
}