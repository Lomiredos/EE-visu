#include "visu/ui/CreateSceneModal.hpp"

#include "visu/helpers/NameValidation.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

CreateSceneModal::CreateSceneModal(std::vector<std::string> _parents,
                                   std::function<void(SceneInfoCreation)> _cb)
{
    m_parents = std::move(_parents);
    m_onClose = _cb;
}

SceneInfoCreation CreateSceneModal::buildResult()
{
    static const char *kHeritages[] = {"public", "protected", "private"};

    SceneInfoCreation res;
    res.name = m_name;
    res.parent = m_parents.empty() ? kDefaultSceneParent
                                   : m_parents[m_selectedParentIdx];
    res.heritage = kHeritages[m_selectedHeritageIdx];
    return res;
}

bool CreateSceneModal::Draw()
{
    bool finished = false;

    if (ImGui::BeginPopupModal(Id(), NULL, 0))
    {
        ImGui::InputTextWithHint("Scene Name", "Your name...", &m_name);
        ImGui::Separator();

        std::vector<const char *> parentLabels;
        for (const std::string &p : m_parents)
            parentLabels.push_back(p.c_str());
        ImGui::Combo("Parent", &m_selectedParentIdx, parentLabels.data(),
                     static_cast<int>(parentLabels.size()));

        static const char *kHeritages[] = {"public", "protected", "private"};
        ImGui::Combo("Heritage", &m_selectedHeritageIdx, kHeritages, 3);

        ImGui::Separator();
        bool nameValid = namevalidation::isValidIdentifier(m_name);
        if (!m_name.empty() && !nameValid)
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                               "Nom invalide (identifiant C++ : lettres/chiffres/_, pas de chiffre en tete).");
        if (!nameValid)
            ImGui::BeginDisabled();
        if (ImGui::Button("Créer"))
        {
            m_onClose(buildResult());
            finished = true;
            ImGui::CloseCurrentPopup();
        }
        if (!nameValid)
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
