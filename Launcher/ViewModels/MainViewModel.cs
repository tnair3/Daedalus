using System.Reactive.Linq;
using System.Reflection;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.Models;
using ReactiveUI;

namespace DaedalusLauncher.ViewModels;

public partial class MainViewModel : ViewModelBase
{
    public string AppVersion => 
        Assembly.GetExecutingAssembly()
            .GetCustomAttribute<AssemblyInformationalVersionAttribute>()?
            .InformationalVersion ?? "1.0.0";
    
    public InstallationsViewModel Installations { get; } = new();
    public LearnViewModel Learn { get; } = new();
    
    public bool IsProjectsTab => SelectedTab == TabType.Projects;
    public bool IsInstallationsTab => SelectedTab == TabType.Installations;
    public bool IsLearnTab => SelectedTab == TabType.Learn;
    
    [ObservableProperty] 
    [NotifyPropertyChangedFor(nameof(IsProjectsTab))]
    [NotifyPropertyChangedFor(nameof(IsInstallationsTab))]
    [NotifyPropertyChangedFor(nameof(IsLearnTab))]
    private TabType _selectedTab;

    // Navigation within the main window
    [RelayCommand]
    public async Task Navigate(TabType targetTab)
    {
        if (targetTab == TabType.Installations && SelectedTab != TabType.Installations)
        {
            await Installations.CheckVersionsAsync();
        }

        if (targetTab == TabType.Learn && SelectedTab != TabType.Learn)
        {
            await Learn.LoadLibraryAsync();
        }
        
        SelectedTab = targetTab;
    }

    // Navigating to open other windows
    [RelayCommand]
    public async Task OpenWindow(WindowType window)
    {
        bool result = window switch
        {
            WindowType.Settings => await RequestDialogAsync(ShowSettingsDialog, new SettingsViewModel()),
            WindowType.Report   => await RequestDialogAsync(ShowReportDialog, new ReportViewModel()),
            WindowType.About    => await RequestDialogAsync(ShowAboutDialog, new AboutViewModel()),
            _                   => await Task.FromResult(false)
        };
    }
    
    private async Task<bool> RequestDialogAsync<TViewModel>(Interaction<TViewModel, bool> interaction, TViewModel vm)
    {
        return await interaction.Handle(vm);
    }
    
    public Interaction<SettingsViewModel, bool> ShowSettingsDialog { get; } = new();
    public Interaction<ReportViewModel, bool> ShowReportDialog { get; } = new();
    public Interaction<AboutViewModel, bool> ShowAboutDialog { get; } = new();
}