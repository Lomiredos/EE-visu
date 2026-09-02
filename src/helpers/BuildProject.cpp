#include "visu/helpers/BuildProject.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace
{
    std::string readAll(const fs::path &_p)
    {
        std::ifstream f(_p, std::ios::binary);
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }
}

namespace projectbuild
{
    BuildResult build(const fs::path &_projectRoot)
    {
        BuildResult res;

        // Chemins natifs, chacun quote -> les espaces ne cassent pas la commande.
        fs::path build = (_projectRoot / "build").make_preferred();
        fs::path log = (build / "editor_build.log").make_preferred();

        const std::string qBuild = "\"" + build.string() + "\"";
        const std::string qLog = "\"" + log.string() + "\"";

        // Cible par defaut (tout), config Debug (convention du projet edite).
        std::string cmd = "cmake --build " + qBuild +
                          " --config Debug > " + qLog + " 2>&1";
        int rc = std::system(cmd.c_str());
        res.log = readAll(log);
        res.ok = (rc == 0);
        return res;
    }
}
