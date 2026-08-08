using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;

namespace DaedalusLauncher.Views;

public partial class LearnView : UserControl
{
    public LearnView()
    {
        InitializeComponent();
    }
    
    private void OnPointerWheelChanged(object? sender, PointerWheelEventArgs e)
    {
        if (sender is ScrollViewer scrollViewer)
        {
            var scrollStep = 50.0;
            var newOffset = scrollViewer.Offset.X - (e.Delta.Y * scrollStep);
            
            scrollViewer.Offset = new Avalonia.Vector(newOffset, 0);
            e.Handled = true;
        }
    }
}