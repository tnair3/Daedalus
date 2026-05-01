using System.Reflection;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class MainViewModel : ViewModelBase
{
    public string AppVersion => 
        Assembly.GetExecutingAssembly()
            .GetCustomAttribute<AssemblyInformationalVersionAttribute>()?
            .InformationalVersion ?? "1.0.0";

    [ObservableProperty] 
    [NotifyPropertyChangedFor(nameof(IsProjectsTab))]
    [NotifyPropertyChangedFor(nameof(IsInstallationsTab))]
    [NotifyPropertyChangedFor(nameof(IsLearnTab))]
    private TabType _selectedTab = TabType.Projects;

    [RelayCommand]
    public void Navigate(TabType targetTab)
    {
        SelectedTab = targetTab;
    }

    public bool IsProjectsTab => SelectedTab == TabType.Projects;
    public bool IsInstallationsTab => SelectedTab == TabType.Installations;
    public bool IsLearnTab => SelectedTab == TabType.Learn;
}