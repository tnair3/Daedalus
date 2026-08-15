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
    
    public NotificationService NotificationService { get; } = new();
    
    [ObservableProperty] private object _currentPage;

    [ObservableProperty]
    [NotifyPropertyChangedFor(nameof(IsProjectsTab))]
    [NotifyPropertyChangedFor(nameof(IsInstallationsTab))]
    [NotifyPropertyChangedFor(nameof(IsLearnTab))]
    [NotifyPropertyChangedFor(nameof(IsAboutTab))]
    [NotifyPropertyChangedFor(nameof(IsReportTab))]
    [NotifyPropertyChangedFor(nameof(IsSettingsTab))]
    private TabType _selectedTab;

    public bool IsProjectsTab => SelectedTab == TabType.Projects;
    public bool IsInstallationsTab => SelectedTab == TabType.Installations;
    public bool IsLearnTab => SelectedTab == TabType.Learn;
    public bool IsAboutTab => SelectedTab == TabType.About;
    public bool IsReportTab => SelectedTab == TabType.Report;
    public bool IsSettingsTab => SelectedTab == TabType.Settings;


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