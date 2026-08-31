#include "visu/helpers/ComponentGen.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    void writeFile(const fs::path &_path, const std::string &_content)
    {
        std::ofstream f(_path, std::ios::binary);
        f << _content;
    }

    // type feuille -> declaration C++ d'un champ avec sa valeur par defaut.
    std::string fieldDecl(const ComponentFieldDef &_f)
    {
        if (_f.type == "int")
            return "    int " + _f.name + " = 0;\n";
        if (_f.type == "bool")
            return "    bool " + _f.name + " = false;\n";
        if (_f.type == "string")
            return "    std::string " + _f.name + ";\n";
        return "    float " + _f.name + " = 0.0f;\n"; // float par defaut
    }

    // (Re)genere RegisterComponents.{hpp,cpp} en listant tous les <Nom>.hpp du
    // dossier (sauf RegisterComponents lui-meme) : un include + un emitComponent.
    void regenerateRegister(const fs::path &_dir)
    {
        std::vector<std::string> names;
        for (const auto &entry : fs::directory_iterator(_dir))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".hpp")
                continue;
            std::string stem = entry.path().stem().string();
            if (stem == "RegisterComponents")
                continue;
            names.push_back(stem);
        }

        writeFile(_dir / "RegisterComponents.hpp",
                  "#pragma once\n\n"
                  "#include <nlohmann/json.hpp>\n\n"
                  "// Genere par EE-Visu : emet chaque composant du projet.\n"
                  "void buildComponentCatalog(nlohmann::json &_components);\n");

        std::ostringstream cpp;
        cpp << "#include \"components/RegisterComponents.hpp\"\n\n";
        cpp << "#include \"visu/reflect/CatalogGen.hpp\"\n\n";
        cpp << "// Genere par EE-Visu -- ne pas editer a la main.\n";
        for (const std::string &n : names)
            cpp << "#include \"components/" << n << ".hpp\"\n";
        cpp << "\nvoid buildComponentCatalog(nlohmann::json &_components)\n{\n";
        for (const std::string &n : names)
            cpp << "    ee::reflection::emitComponent<" << n << ">(_components);\n";
        cpp << "}\n";
        writeFile(_dir / "RegisterComponents.cpp", cpp.str());
    }
}

namespace componentgen
{
    fs::path createComponent(const fs::path &_componentsDir, const ComponentInfoCreation &_def)
    {
        if (_def.name.empty())
            return {};

        std::error_code ec;
        fs::create_directories(_componentsDir, ec);

        const std::string &name = _def.name;

        bool needsString = false;
        for (const auto &f : _def.fields)
            if (f.type == "string")
                needsString = true;

        std::ostringstream hpp;
        hpp << "#pragma once\n\n"
            << "#include \"visu/reflect/Reflect.hpp\"\n"
            << "#include \"visu/reflect/FieldVisitor.hpp\"\n";
        if (needsString)
            hpp << "\n#include <string>\n";
        hpp << "\n// Composant genere par EE-Visu. La struct est la verite ;\n"
            << "// Components.json en est genere via Reflect.\n\n"
            << "struct " << name << "\n{\n";
        for (const auto &f : _def.fields)
            hpp << fieldDecl(f);
        hpp << "};\n\n"
            << "template <>\n"
            << "struct ee::reflection::Reflect<" << name << ">\n{\n"
            << "    static constexpr const char *name = \"" << name << "\";\n\n"
            << "    static void visit(" << name << " &_c, ee::reflection::FieldVisitor &_v)\n"
            << "    {\n";
        for (const auto &f : _def.fields)
            hpp << "        _v.visit(\"" << f.name << "\", _c." << f.name << ");\n";
        hpp << "    }\n};\n";

        fs::path hppPath = _componentsDir / (name + ".hpp");
        writeFile(hppPath, hpp.str());

        regenerateRegister(_componentsDir);

        return hppPath;
    }
}
