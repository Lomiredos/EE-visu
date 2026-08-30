#include "visu/core/SystemInfo.hpp"

#include <nlohmann/json.hpp>

#include <fstream>

std::optional<SystemInfo> loadSystemInfo(const std::filesystem::path &jsonPath)
{
    std::ifstream f(jsonPath);
    if (!f)
        return std::nullopt;

    try
    {
        nlohmann::json j = nlohmann::json::parse(f);

        SystemInfo info;
        info.name = j.value("name", "");
        info.priority = j.value("priority", 0);
        info.category = j.value("category", "");
        if (j.contains("components"))
            info.components = j["components"].get<std::vector<std::string>>();

        return info;
    }
    catch (const std::exception &)
    {
        return std::nullopt; // JSON malformé
    }
}
