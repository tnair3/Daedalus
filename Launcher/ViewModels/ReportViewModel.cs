using System;

namespace DaedalusLauncher.ViewModels;

public class ReportViewModel : ViewModelBase
{
    public event Action<bool>? CloseRequested;

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }
}