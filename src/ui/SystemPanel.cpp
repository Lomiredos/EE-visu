#include "visu/ui/SystemPanel.hpp"

#include "visu/core/Project.hpp"
#include "visu/ui/FolderTree.hpp"
#include "visu/App.hpp"
#include "visu/ui/ShowCodePanel.hpp"

#include "visu/ui/CreateSystemModal.hpp"

#include "visu/helpers/Directions.hpp"

#include "imgui.h"
#include "visu/helpers/OpenExternal.hpp"
#include "visu/helpers/SystemGen.hpp"
#include "visu/helpers/ComponentGetter.hpp"

namespace fs = std::filesystem;

#include <iostream>

void SystemPanel::draw(Project *project)
{
    if (!project || !project->isValid())
    {
        ImGui::TextDisabled("Aucun projet ouvert");
        return;
    }

    fs::path dir = project->systemsDir();
    if (fs::exists(dir))
    {

        if (ImGui::Button("Create new System"))
        {
            // Liste des composants = union moteur + projet, MEME source que
            // l'inspecteur (getAllComponentDefaults). Fini la divergence.
            std::vector<std::string> comps;
            for (const auto &kv : getAllComponentDefaults(project->componentsCatalog()))
                comps.push_back(kv.first);

            App &app = App::getInstance();
            // Genere le trio (.json/.hpp/.cpp) dans <projet>/systems/, rebranche
            // le registre, puis ouvre le stub .cpp dans VS Code.
            app.openModal(std::make_unique<CreateSystemModal>(std::move(comps),
                                                              [dir, this](SystemInfoCreation _data)
                                                              {
                                                                  systemgen::CreateResult r = systemgen::createSystem(dir, _data);
                                                                  if (r.error.empty())
                                                                  {
                                                                      m_status = "Systeme cree.";
                                                                      openInVSCode(r.path);
                                                                  }
                                                                  else
                                                                      m_status = r.error;
                                                              }));
        }

        if (!m_status.empty())
            ImGui::TextDisabled("%s", m_status.c_str());

        ImGui::Separator();

        // Double-click sur un systeme -> ouvre son .cpp dans VS Code.
        fs::path clicked = drawFolderTree(
            dir,
            [](const fs::path &p)
            {
                fs::path cpp = p;
                cpp.replace_extension(".cpp");
                openInVSCode(fs::exists(cpp) ? cpp : p);
            },
            {".hpp"});
        if (!clicked.empty())
        {
            App &app = App::getInstance();

            if (ShowCodePanel *unlocked = app.getPanel<ShowCodePanel>(true))
                unlocked->changePath(clicked);
            else
            {

                std::unique_ptr<Panel> panel = std::make_unique<ShowCodePanel>(clicked);

                if (ShowCodePanel *existing = app.getPanel<ShowCodePanel>(false))
                    panel->dock.dockTarget = existing->dockNode();
                else
                {
                    panel->dock.splitSource = ImGui::GetWindowDockID();
                    panel->dock.splitDir = Dir::Right;
                    panel->dock.splitRatio = 0.5f;
                }

                app.requestPanel(std::move(panel));
            }
        }
    }
    else
        ImGui::Text("Dossier introuvable : %s", dir.string().c_str());
}
