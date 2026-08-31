#pragma once

#include "visu/ui/CreateComponentModal.hpp" // ComponentInfoCreation

#include <filesystem>

// ---------------------------------------------------------------------------
// Generation d'un composant dans le PROJET. Ecrit <projet>/<components>/<Nom>.hpp
// (la struct C++ = la verite + sa specialisation Reflect), puis REGENERE
// RegisterComponents.{hpp,cpp} en scannant le dossier. Au prochain build du jeu,
// le POST_BUILD relance gen_components -> Components.json a jour.
//
// Symetrique de systemgen::createSystem. Un composant de donnees = .hpp seul
// (pas de .cpp : la struct + Reflect sont inline).
// ---------------------------------------------------------------------------

namespace componentgen
{
    // Renvoie le chemin du .hpp genere, ou vide si echec.
    std::filesystem::path createComponent(const std::filesystem::path &_componentsDir,
                                          const ComponentInfoCreation &_def);
}
