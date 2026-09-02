#pragma once

#include <filesystem>
#include <string>

// ---------------------------------------------------------------------------
// Compile le PROJET courant (cible par defaut) via cmake, en capturant la
// sortie du compilo pour l'afficher en cas d'echec. Referme la boucle
// authoring -> build -> Play sans quitter l'editeur.
// NB: bloquant (comme GenerateCatalog) -> l'UI gele le temps de la compilation.
// ---------------------------------------------------------------------------

namespace projectbuild
{
    struct BuildResult
    {
        bool ok = false;
        std::string log; // sortie complete (a montrer si !ok)
    };

    BuildResult build(const std::filesystem::path &_projectRoot);
}
