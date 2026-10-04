#include "visu/helpers/GenerateCatalog.hpp"

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

namespace catalog
{
    GenResult generate(const fs::path &_projectRoot)
    {
        GenResult res;

        // Chemins natifs (backslashes), chacun quote -> les espaces dans le
        // chemin projet ne cassent plus la commande shell.
        fs::path build = (_projectRoot / "build").make_preferred();
        fs::path log = (build / "gen_catalog.log").make_preferred();
#ifdef _WIN32
        fs::path genExe = (build / "gen_components.exe").make_preferred();
#else
        fs::path genExe = (build / "gen_components").make_preferred();
#endif

        const std::string qBuild = "\"" + build.string() + "\"";
        const std::string qLog = "\"" + log.string() + "\"";
        const std::string qGenExe = "\"" + genExe.string() + "\"";

        // 1) Compiler la cible = le test de validite des structs.
        std::string cmd = "cmake --build " + qBuild +
                          " --target gen_components > " +
                          qLog + " 2>&1";
        int rc = std::system(cmd.c_str());
        res.log = readAll(log);
        if (rc != 0)
        {
            res.ok = false;
            return res; // compilation ratee -> le log contient l'erreur
        }

        // 2) Lancer le generateur explicitement (le POST_BUILD ne tourne pas si
        //    rien n'a ete recompile -> on garantit l'ecriture du json).
        std::string runCmd = qGenExe + " >> " + qLog + " 2>&1";
        int rc2 = std::system(runCmd.c_str());
        res.log = readAll(log);
        res.ok = (rc2 == 0);
        return res;
    }
}
