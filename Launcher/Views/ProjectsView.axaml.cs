using System;
using Avalonia.Controls;
using Avalonia.Threading;
using DaedalusLauncher.ViewModels;

namespace DaedalusLauncher.Views;

public partial class ProjectsView : UserControl
{
    public ProjectsView()
    {
        InitializeComponent();
        DataContextChanged += OnDataContextChanged;
    }
    
    private void OnDataContextChanged(object? sender, EventArgs e)
    {
        if (DataContext is ProjectsViewModel vm)
        {
            Dispatcher.UIThread.InvokeAsync(async () => await vm.LoadProjects());
        }
    }
}