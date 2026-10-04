#include "visu/core/Project.hpp"

namespace fs = std::filesystem;

Project::Project(fs::path root) : m_root(std::move(root)) {}

bool Project::isValid() const { return fs::is_directory(m_root); }

std::string Project::name() const { return m_root.filename().string(); }

fs::path Project::systemsDir() const { return m_root / "Systems"; }

fs::path Project::componentsDir() const { return m_root / "Components"; }

fs::path Project::scenesDir() const { return m_root / "Scenes"; }

fs::path Project::componentsCatalog() const {
  return m_root / "assets" / "Components.json";
}

fs::path Project::sceneFile(std::string _sceneName) const {
  return m_root / "Assets/ScenesDatas" / (_sceneName + ".json");
}

fs::path Project::executablePath() const {
#ifdef _WIN32
  return m_root / "build" / (name() + ".exe");
#endif
  return m_root / "build" / name();
}
