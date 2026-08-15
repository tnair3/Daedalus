using Avalonia.ReactiveUI;
using DaedalusLauncher.ViewModels;

namespace DaedalusLauncher.Views;

public partial class MainView : ReactiveWindow<MainViewModel>
{
    public MainView()
    {
        InitializeComponent();

        // About, Report, and Settings are now tabs rendered directly via
        // CurrentPage instead of separate popup windows, so the dialog
        // interaction handlers that used to live here have been removed.
        // CreateProjectView is still shown as a modal dialog from
        // ProjectsView, which is unaffected by this change.
    }
}