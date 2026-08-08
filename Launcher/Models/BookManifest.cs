using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace DaedalusLauncher.Models;

public class BookManifest
{
    [JsonPropertyName("book_id")] public string BookId { get; set; } = string.Empty;
    [JsonPropertyName("title")] public string Title { get; set; } = string.Empty;
    [JsonPropertyName("description")] public string Description { get; set; } = string.Empty;
    [JsonPropertyName("author")] public string Author { get; set; } = string.Empty;
    [JsonPropertyName("version")] public string Version { get; set; } = string.Empty;
    [JsonPropertyName("category")] public string Category { get; set; } = string.Empty;
    [JsonPropertyName("shelf_index")] public int ShelfIndex { get; set; }
    [JsonPropertyName("chapters")] public List<ChapterInfo> Chapters { get; set; } = new();
}

public class ChapterInfo
{
    [JsonPropertyName("title")] public string Title { get; set; } = string.Empty;
    [JsonPropertyName("file")] public string FileName { get; set; } = string.Empty;
}