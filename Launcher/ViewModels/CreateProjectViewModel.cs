using System;

namespace DaedalusLauncher.ViewModels;

public partial class CreateProjectViewModel : ViewModelBase
{
    public event Action<bool>? CloseRequested;

    private void RequestClose(bool result)
    {
        CloseRequested?.Invoke(result);
    }
}