#include "visu/ui/CreateSystemModal.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "visu/helpers/ComponentGetter.hpp"

#include <iostream>
CreateSystemModal::CreateSystemModal(std::function<void(SystemInfoCreation)> _cb)
{
    m_componentsNames = getAvaibleComponents();
    for (size_t i = 0; i < m_componentsNames.size(); i++)
    {
        m_valide.push_back(false);
    }
    m_onClose = _cb;
}

SystemInfoCreation CreateSystemModal::buildResult()
{

    std::vector<std::string> noms;
    for (size_t i = 0; i < m_componentsNames.size(); i++)
        if (m_valide[i])
            noms.push_back(m_componentsNames[i]);

    return SystemInfoCreation{m_sysName, noms};
}

bool CreateSystemModal::Draw()
{
    bool finished = false;

    if (ImGui::BeginPopupModal(Id(), NULL, 0))
    {
        ImGui::InputTextWithHint("System Name", "Your name...", &m_sysName);
        ImGui::Separator();
        ImGui::Text("Required Components:");
        for (size_t i = 0; i < m_componentsNames.size(); i++)
        {
            bool coche = m_valide[i];
            if (ImGui::Checkbox(m_componentsNames[i].c_str(), &coche))
                m_valide[i] = coche;
        }
        ImGui::Separator();
        bool isnameValide = !m_sysName.empty();
        if (!isnameValide)
            ImGui::BeginDisabled();
        if (ImGui::Button("Créer"))
        {
            m_onClose(buildResult());
            finished = true;
            ImGui::CloseCurrentPopup();
        }
        if (!isnameValide)
            ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Annuler"))
        {
            ImGui::CloseCurrentPopup();
            finished = true;
        }
        ImGui::EndPopup();
    }

    return finished;
}
