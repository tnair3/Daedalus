using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using DaedalusLauncher.ViewModels;

namespace DaedalusLauncher.Views;

public partial class ReportView : Window
{
    public ReportView()
    {
        InitializeComponent();
    }
    
    protected override void OnDataContextChanged(EventArgs e)
    {
        base.OnDataContextChanged(e);

        if (DataContext is AboutViewModel vm)
        {
            vm.CloseRequested += (result) =>
            {
                this.Close(result);
            };
        }
    }
}