using System;
using System.Reactive.Disposables;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.ReactiveUI;
using DaedalusLauncher.ViewModels;
using ReactiveUI;

namespace DaedalusLauncher.Views;

public partial class MainView : ReactiveWindow<MainViewModel>
{
    public MainView()
    {
        InitializeComponent();
        
        this.WhenActivated((CompositeDisposable disposables) =>
        {
            this.ViewModel!.ShowSettingsDialog
                .RegisterHandler(interaction => ShowDialogAsync<SettingsView, SettingsViewModel>(interaction))
                .DisposeWith(disposables);

            this.ViewModel!.ShowReportDialog
                .RegisterHandler(interaction => ShowDialogAsync<ReportView, ReportViewModel>(interaction))
                .DisposeWith(disposables);
            
            this.ViewModel!.ShowAboutDialog
                .RegisterHandler(interaction => ShowDialogAsync<AboutView, AboutViewModel>(interaction))
                .DisposeWith(disposables);
        });
    }
    
    private async Task ShowDialogAsync<TWindow, TViewModel>(IInteractionContext<TViewModel, bool> interaction) 
        where TWindow : Window, new()
    {
        var dialog = new TWindow
        {
            DataContext = interaction.Input
        };
        
        var result = await dialog.ShowDialog<bool>(this);
        interaction.SetOutput(result);
    }
}