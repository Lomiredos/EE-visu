#include "visu/core/Project.hpp"

namespace fs = std::filesystem;

Project::Project(fs::path root) : m_root(std::move(root)) {}

bool Project::isValid() const { return fs::is_directory(m_root); }

std::string Project::name() const { return m_root.filename().string(); }

fs::path Project::systemsDir() const { return m_root / "Systems"; }

fs::path Project::componentsDir() const { return m_root / "Components"; }

fs::path Project::componentsCatalog() const {
  return m_root / "assets" / "Components.json";
}

fs::path Project::sceneFile() const { return m_root / "assets" / "scene.json"; }

fs::path Project::executablePath() const {
  // Convention MSVC multi-config : build/Debug/<nom>.exe
  return m_root / "build" / "Debug" / (name() + ".exe");
}
