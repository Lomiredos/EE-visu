#pragma once

#include <filesystem>
#include <string>

class Project
{
public:
    explicit Project(std::filesystem::path root);

    bool isValid() const;                     
    std::string name() const;                 

    const std::filesystem::path &root() const { return m_root; }
    std::filesystem::path systemsDir() const;
    std::filesystem::path componentsDir() const;
    std::filesystem::path componentsCatalog() const; // assets/Components.json
    std::filesystem::path sceneFile() const;         // assets/scene.json
    std::filesystem::path executablePath() const;    // build/Debug/<name>.exe (MSVC)

private:
    std::filesystem::path m_root;
};
