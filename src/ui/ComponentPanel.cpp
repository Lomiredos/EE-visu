#include "visu/ui/ComponentPanel.hpp"

#include "visu/core/Project.hpp"
#include "visu/ui/FolderTree.hpp"

#include "imgui.h"
#include "visu/helpers/OpenExternal.hpp"

#include "visu/App.hpp"
#include "visu/ui/ShowCodePanel.hpp"

#include "visu/ui/CreateComponentModal.hpp"
#include "visu/helpers/ComponentGen.hpp"
#include "visu/helpers/GenerateCatalog.hpp"

namespace fs = std::filesystem;

void ComponentPanel::draw(Project *project)
{
    if (!project || !project->isValid())
    {
        ImGui::TextDisabled("Aucun projet ouvert");
        return;
    }

    fs::path dir = project->componentsDir();

    if (ImGui::Button("Create new Component"))
    {
        App &app = App::getInstance();
        // Genere <projet>/Components/<Nom>.hpp (struct + Reflect), regenere
        // RegisterComponents, puis ouvre le .hpp dans VS Code.
        app.openModal(std::make_unique<CreateComponentModal>([dir, this](ComponentInfoCreation _data)
                                                             {
                                                                 componentgen::CreateResult r = componentgen::createComponent(dir, _data);
                                                                 if (r.error.empty())
                                                                 {
                                                                     m_status = "Composant cree.";
                                                                     openInVSCode(r.path);
                                                                 }
                                                                 else
                                                                     m_status = r.error;
                                                             }));
    }

    ImGui::SameLine();

    // Generation par VRAIE compilation : compile gen_components (test de
    // validite des structs), puis ecrit Components.json ; sinon popup d'erreur.
    if (ImGui::Button("Generate catalog (build)"))
    {
        catalog::GenResult r = catalog::generate(project->root());
        if (r.ok)
            m_status = "Catalogue genere.";
        else
        {
            m_status = "Echec de la compilation.";
            m_buildLog = r.log;
            m_openErrorPopup = true;
        }
    }
    if (!m_status.empty())
        ImGui::TextDisabled("%s", m_status.c_str());

    // Popup d'erreur (bloquant) : montre la sortie du compilo.
    if (m_openErrorPopup)
    {
        ImGui::OpenPopup("Erreur de compilation");
        m_openErrorPopup = false;
    }
    if (ImGui::BeginPopupModal("Erreur de compilation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("La generation a echoue. Sortie :");
        ImGui::BeginChild("log", ImVec2(720, 320), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(m_buildLog.c_str());
        ImGui::EndChild();
        if (ImGui::Button("Fermer"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Separator();

    auto openHpp = [](const fs::path &p)
    { openInVSCode(p); };

    // Composants du PROJET (on masque les fichiers de registration).
    ImGui::TextDisabled("Projet");
    fs::path clicked;
    if (fs::exists(dir))
        clicked = drawFolderTree(dir, openHpp, {".hpp"},
                                 {"RegisterComponents", "RegisterWorldComponents", "Components"});
    else
        ImGui::TextDisabled("(aucun composant -- cree-en un)");

    // Composants MOTEUR (ee-core, submodule) -- lecture seule. On masque
    // l'agregateur Components.hpp (ce n'est pas un composant).
    fs::path engineDir = fs::path(ASSETS_DIR).parent_path() /
                         "extern" / "eliott-engine-3d" / "include" / "visu" / "components";
    ImGui::TextDisabled("Moteur");
    fs::path clickedEngine = drawFolderTree(engineDir, openHpp, {".hpp"}, {"Components"});
    if (clicked.empty())
        clicked = clickedEngine;

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
