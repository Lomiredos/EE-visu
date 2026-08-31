#pragma once

#include "visu/ui/CreateSystemModal.hpp" // SystemInfoCreation

#include <filesystem>

// ---------------------------------------------------------------------------
// Generation d'un systeme dans le PROJET (les systemes n'appartiennent pas au
// moteur). Ecrit, dans <projet>/systems/, le trio de fichiers d'un systeme :
//   <Nom>.json  : le sidecar (signature, priorite) lu par le runtime + l'editeur
//   <Nom>.hpp   : la declaration, convention uniforme void <Nom>(FrameContext&)
//   <Nom>.cpp   : le stub a completer (le corps du systeme, ouvert dans VS Code)
// Puis REGENERE RegisterSystems.{hpp,cpp} (nom -> fonction) en scannant le
// dossier -> un systeme cree est automatiquement branche au registre du jeu.
// ---------------------------------------------------------------------------

namespace systemgen
{
    // Renvoie le chemin du .cpp genere (a ouvrir), ou un chemin vide si echec.
    std::filesystem::path createSystem(const std::filesystem::path &_systemsDir,
                                       const SystemInfoCreation &_def);
}
