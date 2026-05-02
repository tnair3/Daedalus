using System;

namespace DaedalusLauncher.ViewModels;

public class AboutViewModel : ViewModelBase
{
    public event Action<bool>? CloseRequested;

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }
}