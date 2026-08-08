using System.Windows.Input;
using Avalonia;
using Avalonia.Controls.Primitives;
using Avalonia.Metadata;

namespace DaedalusLauncher.Controls;

public class WindowFrame : TemplatedControl
{
    // Define the Content property so the control can hold inner elements
    public static readonly StyledProperty<object?> ContentProperty =
        AvaloniaProperty.Register<WindowFrame, object?>(nameof(Content));

    [Content]
    public object? Content
    {
        get => GetValue(ContentProperty);
        set => SetValue(ContentProperty, value);
    }

    // Define the WindowName property
    public static readonly StyledProperty<string> WindowNameProperty =
        AvaloniaProperty.Register<WindowFrame, string>(nameof(WindowName), "Window Name");

    public string WindowName
    {
        get => GetValue(WindowNameProperty);
        set => SetValue(WindowNameProperty, value);
    }

    // Define the CloseWindowCommand property
    public static readonly StyledProperty<ICommand?> CloseWindowCommandProperty =
        AvaloniaProperty.Register<WindowFrame, ICommand?>(nameof(CloseWindowCommand));

    public ICommand? CloseWindowCommand
    {
        get => GetValue(CloseWindowCommandProperty);
        set => SetValue(CloseWindowCommandProperty, value);
    }
}