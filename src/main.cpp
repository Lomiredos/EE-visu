#include "visu/App.hpp"

#include <cstdio>
#include <filesystem>

int main()
{
    App &app = App::getInstance();

    if (!app.init())
        return 1;

    // TODO: remplacer par un selecteur de projet. En attendant, chemin fixe.
    const std::filesystem::path projectPath = "C:/Dev/eliott-engine-projects/empty-sphere";
    if (!std::filesystem::is_directory(projectPath))
        std::fprintf(stderr,
                     "[EE-Visu] Projet introuvable : %s\n"
                     "L'editeur demarre sans projet (panneaux vides).\n",
                     projectPath.string().c_str());
    app.openProject(projectPath);

    app.run();

    return 0;
}
