#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct SystemInfo
{
    std::string name;
    std::vector<std::string> components;
    int priority = 0;
    std::string category; // "gameplay" | "ui"
};

// Charge <chemin>.json. Renvoie nullopt si le fichier est absent ou illisible.
std::optional<SystemInfo> loadSystemInfo(const std::filesystem::path &jsonPath);
