#include "visu/helpers/SystemGen.hpp"

#include "visu/core/SystemInfo.hpp" // loadSystemInfo (pour scanner les .json)

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    // "A", "B", "C" pour un initializer-list C++.
    std::string quotedList(const std::vector<std::string> &_names)
    {
        std::string out;
        for (size_t i = 0; i < _names.size(); ++i)
        {
            if (i)
                out += ", ";
            out += "\"" + _names[i] + "\"";
        }
        return out;
    }

    void writeFile(const fs::path &_path, const std::string &_content)
    {
        std::ofstream f(_path, std::ios::binary);
        f << _content;
    }

    // (Re)genere RegisterSystems.{hpp,cpp} en listant tous les <Nom>.json du
    // dossier : un include + un registry.add par systeme. Le main du jeu ne
    // bouge jamais, seul ce fichier est reecrit.
    void regenerateRegister(const fs::path &_systemsDir)
    {
        std::vector<std::string> names;
        for (const auto &entry : fs::directory_iterator(_systemsDir))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".json")
                continue;
            if (auto info = loadSystemInfo(entry.path()))
                if (!info->name.empty())
                    names.push_back(info->name);
        }

        writeFile(_systemsDir / "RegisterSystems.hpp",
                  "#pragma once\n\n"
                  "#include \"visu/systems/SystemScheduler.hpp\"\n\n"
                  "// Genere par EE-Visu. Branche tous les systemes du projet.\n"
                  "void registerGameSystems(ee::systems::SystemRegistry &_registry);\n");

        std::ostringstream cpp;
        cpp << "#include \"systems/RegisterSystems.hpp\"\n\n";
        cpp << "// Genere par EE-Visu -- ne pas editer a la main.\n";
        for (const std::string &n : names)
            cpp << "#include \"systems/" << n << ".hpp\"\n";
        cpp << "\nvoid registerGameSystems(ee::systems::SystemRegistry &_registry)\n{\n";
        for (const std::string &n : names)
            cpp << "    _registry.add(\"" << n << "\", &" << n << ");\n";
        cpp << "}\n";
        writeFile(_systemsDir / "RegisterSystems.cpp", cpp.str());
    }
}

namespace systemgen
{
    fs::path createSystem(const fs::path &_systemsDir, const SystemInfoCreation &_def)
    {
        if (_def.name.empty())
            return {};

        std::error_code ec;
        fs::create_directories(_systemsDir, ec);

        const std::string &name = _def.name;

        // --- 1) sidecar .json (schema SystemInfo) ---
        nlohmann::json j;
        j["name"] = name;
        j["components"] = _def.requiredComponentName;
        j["priority"] = 0;
        j["category"] = "gameplay";
        writeFile(_systemsDir / (name + ".json"), j.dump(2) + "\n");

        // --- 2) header (convention uniforme) ---
        std::ostringstream hpp;
        hpp << "#pragma once\n\n"
            << "#include \"visu/systems/SystemScheduler.hpp\"\n\n"
            << "// Systeme genere par EE-Visu.\n"
            << "// Convention : void <Nom>(const ee::systems::FrameContext &).\n"
            << "void " << name << "(const ee::systems::FrameContext &_ctx);\n";
        writeFile(_systemsDir / (name + ".hpp"), hpp.str());

        // --- 3) stub .cpp (le corps a completer) ---
        std::ostringstream cpp;
        cpp << "#include \"systems/" << name << ".hpp\"\n\n"
            << "#include \"visu/core/SceneQuery.hpp\"\n\n"
            << "void " << name << "(const ee::systems::FrameContext &_ctx)\n"
            << "{\n"
            << "    SceneInfo &scene = *_ctx.scene;\n\n"
            << "    // Signature : les entites traitees par ce systeme.\n"
            << "    static const std::vector<std::string> kSignature = { "
            << quotedList(_def.requiredComponentName) << " };\n\n"
            << "    for (EntityInfo &ent : scene.entities)\n"
            << "    {\n"
            << "        if (!ee::scene::matchesSignature(ent, kSignature))\n"
            << "            continue;\n\n"
            << "        // TODO: ta logique ici. Exemple :\n"
            << "        //   ComponentInstance *tf = ee::scene::findComponent(ent, \"TransformComponent\");\n"
            << "        //   float x = ee::scene::getFloat(*tf, \"x\", 0.0f);\n"
            << "        //   ee::scene::setFloat(*tf, \"x\", x + _ctx.input.moveX * _ctx.dt);\n"
            << "    }\n"
            << "}\n";
        fs::path cppPath = _systemsDir / (name + ".cpp");
        writeFile(cppPath, cpp.str());

        // --- 4) rebrancher tout au registre ---
        regenerateRegister(_systemsDir);

        return cppPath;
    }
}
