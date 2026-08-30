#include "visu/ui/ComponentPanel.hpp"

#include "visu/core/Project.hpp"
#include "visu/ui/FolderTree.hpp"

#include "imgui.h"
#include "visu/helpers/OpenExternal.hpp"

#include "visu/App.hpp"
#include "visu/ui/ShowCodePanel.hpp"

namespace fs = std::filesystem;

void ComponentPanel::draw(Project *project)
{
    if (!project || !project->isValid())
    {
        ImGui::TextDisabled("Aucun projet ouvert");
        return;
    }

    fs::path dir = project->componentsDir();
    if (fs::exists(dir))
    {
        fs::path clicked = drawFolderTree(dir, openInDefaultApp, {".hpp"});
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
