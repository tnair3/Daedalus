using System.Reflection;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.Controls;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class MainViewModel : ViewModelBase
{
    public string AppVersion =>
        Assembly.GetExecutingAssembly()
            .GetCustomAttribute<AssemblyInformationalVersionAttribute>()?
            .InformationalVersion ?? "1.0.0";
    
    private ProjectsViewModel Projects { get; } = new();
    private InstallationsViewModel Installations { get; } = new();
    private LearnViewModel Learn { get; } = new();
    private AboutViewModel About { get; } = new();
    private ReportViewModel Report { get; } = new();
    private SettingsViewModel Settings { get; } = new();
    
    [ObservableProperty] private ViewModelBase _currentPage;
    [ObservableProperty] private TabType _selectedTab;

    public NotificationService NotificationService { get; } = new();

    public MainViewModel()
    {
        _selectedTab = TabType.Projects;
        _currentPage = Projects;
    }


    [RelayCommand]
    private async Task Navigate(TabType targetTab)
    {
        bool alreadyOnTab = SelectedTab == targetTab;
        SelectedTab = targetTab;

        switch (targetTab)
        {
            case TabType.Projects:
                CurrentPage = Projects;
                break;

            case TabType.Installations:
                CurrentPage = Installations;
                if (!alreadyOnTab)
                {
                    await Installations.CheckVersionsAsync();
                }
                break;

            case TabType.Learn:
                CurrentPage = Learn;
                if (!alreadyOnTab)
                {
                    await Learn.LoadLibraryAsync();
                }
                break;

            case TabType.About:
                CurrentPage = About;
                break;

            case TabType.Report:
                CurrentPage = Report;
                break;

            case TabType.Settings:
                CurrentPage = Settings;
                break;
        }
    }
}