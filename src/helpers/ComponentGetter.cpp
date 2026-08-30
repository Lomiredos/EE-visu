#include "visu/helpers/ComponentGetter.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

#include <iostream>

constexpr const char *enginePath = "assets/Components.json";

std::vector<std::string> getAvaibleComponents(const fs::path &_otherPath)
{
    std::vector<std::string> names = getName(enginePath);
    if (_otherPath != "" && fs::exists(_otherPath) && _otherPath.extension() == ".json")
    {
        std::vector<std::string> externalName = getName(_otherPath);
        names.insert(names.end(), std::make_move_iterator(externalName.begin()), std::make_move_iterator(externalName.end()));
    }

    return names;
}

std::vector<std::string> getName(const fs::path &_filePath)
{
    std::ifstream f(_filePath);
    json data = json::parse(f);
    std::vector<std::string> names;
    for (const auto &comp : data["components"])
    {
        names.push_back(comp["name"]);
    }
    return names;
}

std::map<std::string, std::map<std::string, FieldValue>> getComponentDefaults(const fs::path &_catalog)
{
    std::map<std::string, std::map<std::string, FieldValue>> out;

    std::ifstream f(_catalog);
    if (!f)
        return out;

    try
    {
        json data = json::parse(f);
        for (const auto &comp : data["components"])
        {
            std::string cname = comp.value("name", "");
            if (cname.empty())
                continue;

            std::map<std::string, FieldValue> fields;
            if (comp.contains("fields"))
                for (const auto &fld : comp["fields"])
                {
                    std::string fname = fld.value("name", "");
                    if (fname.empty())
                        continue;

                    const std::string ftype = fld.value("type", "float");
                    if (ftype == "bool")
                        fields[fname] = fld.value("default", false);
                    else if (ftype == "int")
                        fields[fname] = fld.value("default", 0);
                    else if (ftype == "string")
                        fields[fname] = fld.value("default", std::string{});
                    else
                        fields[fname] = fld.value("default", 0.0f);
                }
            out[cname] = std::move(fields);
        }
    }
    catch (const std::exception &)
    {
        // catalogue absent/malforme -> map vide
    }

    return out;
}

std::map<std::string, std::map<std::string, FieldValue>> getAllComponentDefaults(const fs::path &_projectCatalog)
{
    auto all = getComponentDefaults(std::string(ASSETS_DIR) + "/Components.json");

    auto project = getComponentDefaults(_projectCatalog);

    all.merge(project);
    for (const auto &kv : project)
        std::cerr << "[Components] nom deja defini par le standard, version projet ignoree : "
                  << kv.first << '\n';

    return all;
}
