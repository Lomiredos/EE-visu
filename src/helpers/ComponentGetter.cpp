#include "visu/helpers/ComponentGetter.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

#include <iostream>

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
