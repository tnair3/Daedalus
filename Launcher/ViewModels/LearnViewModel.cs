using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading.Tasks;
using Avalonia.Threading;
using DaedalusLauncher.Models;

namespace DaedalusLauncher.ViewModels;

public partial class LearnViewModel : ViewModelBase
{
    public ObservableCollection<BookViewModel> ApplicationBooks { get; } = new();
    public ObservableCollection<BookViewModel> SoftwareBooks { get; } = new();

    public LearnViewModel()
    {
        _ = LoadLibraryAsync();
    }

    public async Task LoadLibraryAsync()
    {
        Dispatcher.UIThread.Post(() => 
        {
            ApplicationBooks.Clear();
            SoftwareBooks.Clear();
        });
        
        string booksRoot = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "../Books");
        
        await Task.Run(() => 
        {
            var bookFolders = Directory.GetDirectories(booksRoot, "*", SearchOption.AllDirectories);
            
            var allBooks = new List<BookViewModel>();

            foreach (var folder in bookFolders)
            {
                string manifestPath = Path.Combine(folder, "manifest.json");
                if (File.Exists(manifestPath))
                {
                    var json = File.ReadAllText(manifestPath);
                    var manifest = JsonSerializer.Deserialize<BookManifest>(json, new JsonSerializerOptions { PropertyNameCaseInsensitive = true });
                    
                    if (manifest != null)
                        allBooks.Add(new BookViewModel(manifest, folder));
                }
            }
            
            var sorted = allBooks
                .OrderBy(b => b.Manifest?.ShelfIndex ?? int.MaxValue)
                .ToList();
            
            Dispatcher.UIThread.Post(() => {
                foreach (var book in sorted)
                {
                    if (book.Category == "Application Basics")
                        ApplicationBooks.Add(book);
                    else if (book.Category == "Software Architecture")
                        SoftwareBooks.Add(book);
                }
            });
        });
    }
}