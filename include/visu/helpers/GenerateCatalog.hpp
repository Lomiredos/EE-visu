#pragma once

#include <filesystem>
#include <string>

// ---------------------------------------------------------------------------
// Genere Components.json du projet par VRAIE compilation :
//   1. compile la cible gen_components (inclut les structs de composants) ;
//      si une struct est invalide, la compilation echoue -> c'est le "test".
//   2. si OK, lance gen_components -> ecrit assets/Components.json.
// La sortie (compilo + generateur) est capturee pour l'afficher en cas d'echec.
// ---------------------------------------------------------------------------

namespace catalog
{
    struct GenResult
    {
        bool ok = false;
        std::string log; // sortie complete (a montrer si !ok)
    };

    GenResult generate(const std::filesystem::path &_projectRoot);
}
