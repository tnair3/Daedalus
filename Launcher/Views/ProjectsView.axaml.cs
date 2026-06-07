using System;
using Avalonia.Controls;
using Avalonia.ReactiveUI;
using Avalonia.Threading;
using DaedalusLauncher.ViewModels;
using System.Threading.Tasks;
using System.Reactive.Disposables;
using ReactiveUI;

namespace DaedalusLauncher.Views;

public partial class ProjectsView : ReactiveUserControl<ProjectsViewModel>
{
    public ProjectsView()
    {
        InitializeComponent();
        DataContextChanged += OnDataContextChanged;
        
        this.WhenActivated((disposables) =>
        {
            this.ViewModel!.ShowCreateProjectDialog
                .RegisterHandler(DoOpenCreateProjectDialog)
                .DisposeWith(disposables);
        });
    }
    
    private void OnDataContextChanged(object? sender, EventArgs e)
    {
        if (DataContext is ProjectsViewModel vm)
        {
            Dispatcher.UIThread.InvokeAsync(async () => await vm.LoadProjects());
        }
    }

    private async Task DoOpenCreateProjectDialog(IInteractionContext<CreateProjectViewModel, bool> context)
    {
        var parentWindow = TopLevel.GetTopLevel(this) as Window;
        
        var dialog = new CreateProjectView
        {
            DataContext = context.Input
        };
        
        if (parentWindow != null)
        {
            var result = await dialog.ShowDialog<bool>(parentWindow);
            context.SetOutput(result);
        }
        else
        {
            context.SetOutput(false);
        }
    }
}