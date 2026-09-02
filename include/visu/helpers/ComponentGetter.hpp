#pragma once

#include <map>
#include <string>
#include <vector>
#include <filesystem>

#include "visu/core/SceneInfo.hpp" // FieldValue

namespace fs = std::filesystem;

// Lit un catalogue Components.json et renvoie, par composant, ses champs avec
// leur valeur par defaut, TYPEE selon le "type" declare dans le catalogue :
// { "TransformComponent": { "x": 0.0f, ... }, "TagComponent": { "name": "" } }
std::map<std::string, std::map<std::string, FieldValue>> getComponentDefaults(const fs::path &_catalog);

// Union des composants STANDARD (catalogue moteur, ASSETS_DIR/Components.json)
// et des composants du PROJET (_projectCatalog). Le standard est prioritaire :
// en cas de nom commun, la version projet est ignoree et signalee (stderr).
std::map<std::string, std::map<std::string, FieldValue>> getAllComponentDefaults(const fs::path &_projectCatalog);
