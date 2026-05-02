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

    [ObservableProperty] 
    [NotifyPropertyChangedFor(nameof(IsProjectsTab))]
    [NotifyPropertyChangedFor(nameof(IsInstallationsTab))]
    [NotifyPropertyChangedFor(nameof(IsLearnTab))]
    private TabType _selectedTab = TabType.Projects;

    [RelayCommand]
    public async Task Navigate(TabType targetTab)
    {
        if (targetTab == TabType.Installations && SelectedTab != TabType.Installations)
        {
            await Installations.CheckVersionsAsync();
        }
        
        SelectedTab = targetTab;
    }
    
    private async Task<bool> RequestDialogAsync<TViewModel>(Interaction<TViewModel, bool> interaction, TViewModel vm)
    {
        return await interaction.Handle(vm);
    }
    
    [RelayCommand]
    public async Task OpenSettings()
    {
        await RequestDialogAsync(ShowSettingsDialog, new SettingsViewModel());
    }
    
    [RelayCommand]
    public async Task OpenReport()
    {
        await RequestDialogAsync(ShowReportDialog, new ReportViewModel());
    }
    
    [RelayCommand]
    public async Task OpenAbout()
    {
        await RequestDialogAsync(ShowAboutDialog, new AboutViewModel());
    }
    
    public Interaction<SettingsViewModel, bool> ShowSettingsDialog { get; } = new();
    public Interaction<ReportViewModel, bool> ShowReportDialog { get; } = new();
    public Interaction<AboutViewModel, bool> ShowAboutDialog { get; } = new();

    public bool IsProjectsTab => SelectedTab == TabType.Projects;
    public bool IsInstallationsTab => SelectedTab == TabType.Installations;
    public bool IsLearnTab => SelectedTab == TabType.Learn;
}