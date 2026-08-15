using System;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class BookViewModel : ViewModelBase
{
    public string Title { get; }
    public string Description { get; }
    public string Author { get; }
    public string Category { get; }
    public string Version { get; }
    public BookManifest? Manifest { get; }

    public BookViewModel(BookManifest manifest, string absoluteFolderPath)
    {
        Manifest = manifest;
        Title = manifest.Title;
        Description = manifest.Description;
        Author = manifest.Author;
        Category = manifest.Category;
        Version = manifest.Version;
    }

    [RelayCommand]
    private void OpenBook()
    {
        // TODO: Implement the reader to be opened
        Console.WriteLine($"Opening: {Title}");
    }
}