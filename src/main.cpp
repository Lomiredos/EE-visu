#include "visu/App.hpp"

#include <cstdio>
#include <filesystem>

int main(int argc, char *argv[]) {
  App &app = App::getInstance();

  if (!app.init())
    return 1;

  const std::filesystem::path projectPath =
      argc > 1 ? std::filesystem::path(argv[1])
               : "C:/Dev/eliott-engine-projects/empty-sphere";
  if (!std::filesystem::is_directory(projectPath))
    std::fprintf(stderr,
                 "[EE-Visu] Projet introuvable : %s\n"
                 "L'editeur demarre sans projet (panneaux vides).\n",
                 projectPath.string().c_str());
  app.openProject(projectPath);

  app.run();

  return 0;
}
