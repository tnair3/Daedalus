using System.Collections.Generic;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Platform.Storage;
using DaedalusLauncher.ViewModels;

namespace DaedalusLauncher.Views;

public partial class CreateProjectView : UserControl
{
    public CreateProjectView()
    {
        InitializeComponent();
    }
    
    private async void SelectFolderButton_Click(object? sender, RoutedEventArgs e)
    {
        var topLevel = TopLevel.GetTopLevel(this);
        if (topLevel == null) return;
        
        IReadOnlyList<IStorageFolder> folders = await topLevel.StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
        {
            Title = "Select Project Directory",
            AllowMultiple = false
        });
        
        if (folders.Count > 0)
        {
            string selectedPath = folders[0].Path.LocalPath;
            
            if (DataContext is CreateProjectViewModel vm)
            {
                vm.ProjectPath = selectedPath;
            }
        }
    }
}