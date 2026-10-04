#pragma once

#include "visu/ui/CreateSceneModal.hpp" // SceneInfoCreation

#include <filesystem>
#include <string>

// ---------------------------------------------------------------------------
// Generation d'une scene dans le PROJET (Scenes/). Ecrit <Nom>.hpp : la
// classe herite de son parent (ee::Scene ou une autre scene du projet, deja
// voisine dans Scenes/ -> include relatif simple) et stub les 5 hooks
// virtuels de ee::Scene (onEnter/onExit/onEvent/onUpdate/onRender), les
// seuls que le moteur expose -- pas besoin d'introspection du parent.
// ---------------------------------------------------------------------------

namespace scenegen
{
    // Resultat de creation : chemin du .hpp genere si succes (error vide),
    // sinon path vide + message d'erreur (nom invalide, collision, ecriture KO).
    struct CreateResult
    {
        std::filesystem::path path;
        std::string error;
    };

    CreateResult createScene(const std::filesystem::path &_scenesDir,
                             const SceneInfoCreation &_def);
}
