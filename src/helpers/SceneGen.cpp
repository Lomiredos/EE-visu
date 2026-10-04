#include "visu/helpers/SceneGen.hpp"

#include "visu/helpers/NameValidation.hpp"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace {
bool writeFile(const fs::path &_path, const std::string &_content) {
  std::ofstream f(_path, std::ios::binary);
  if (!f)
    return false;
  f << _content;
  return f.good();
}
} // namespace

namespace scenegen {
CreateResult createScene(const fs::path &_scenesDir,
                         const SceneInfoCreation &_def) {
  if (!namevalidation::isValidIdentifier(_def.name))
    return {{}, "Nom invalide (identifiant C++ requis)."};

  const std::string &name = _def.name;

  fs::path hppPath = _scenesDir / (name + ".hpp");
  if (fs::exists(hppPath))
    return {{}, "\"" + name + "\" existe deja."};

  std::error_code ec;
  fs::create_directories(_scenesDir, ec);

  const bool defaultParent = _def.parent == kDefaultSceneParent;
  const std::string includePath =
      defaultParent ? "engine/Scene.hpp" : (_def.parent + ".hpp");
  const std::string baseClass = defaultParent ? "ee::Scene" : _def.parent;

  std::ostringstream hpp;
  hpp << "#pragma once\n\n"
      << "#include \"" << includePath << "\"\n\n"
      << "// Scene generee par EE-Visu. Attention a ce que tu touche hein !\n"
      << "class " << name << " : " << _def.heritage << " " << baseClass << "\n"
      << "{\n"
      << "public:\n"
      << "    " << name << "(ee::math::Rect<float> _bounds) : " << baseClass
      << "(_bounds) {}\n\n"
      << "    void onEnter(ee::renderer::Renderer &_renderer) override {}\n"
      << "    void onExit() override {}\n"
      << "    void onEvent(SDL_Event &_e) override {}\n"
      << "    void onUpdate(float _dt) override {}\n"
      << "    void onRender(ee::renderer::Renderer &_renderer) override {}\n"
      << "};\n";
  if (!writeFile(hppPath, hpp.str()))
    return {{}, "Ecriture impossible (fichier verrouille ?)."};

  return {hppPath, {}};
}
} // namespace scenegen
