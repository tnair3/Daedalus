using System.Reactive.Linq;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.Input;
using DaedalusLauncher.Models;
using ReactiveUI;

namespace DaedalusLauncher.ViewModels;

public partial class ProjectsViewModel : ViewModelBase
{
    public Interaction<CreateProjectViewModel, bool> ShowCreateProjectDialog { get; } = new();

    [RelayCommand]
    public async Task OpenCreateProjectWindow()
    {
        var createProjectVm = new CreateProjectViewModel();
        
        bool isCreated = await ShowCreateProjectDialog.Handle(createProjectVm);
        
        if (isCreated)
        {
            // Logic to refresh project list, etc.
        }
    }
}