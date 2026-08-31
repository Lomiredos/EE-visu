#include "visu/ui/CreateComponentModal.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

CreateComponentModal::CreateComponentModal(std::function<void(ComponentInfoCreation)> _cb)
{
    m_onClose = _cb;
    m_fields.push_back({"", "float"}); // un champ vide par defaut
}

ComponentInfoCreation CreateComponentModal::buildResult()
{
    ComponentInfoCreation res;
    res.name = m_name;
    for (const auto &f : m_fields)
        if (!f.name.empty())
            res.fields.push_back(f);
    return res;
}

bool CreateComponentModal::Draw()
{
    bool finished = false;

    if (ImGui::BeginPopupModal(Id(), NULL, 0))
    {
        ImGui::InputTextWithHint("Component Name", "Your name...", &m_name);
        ImGui::Separator();
        ImGui::TextUnformatted("Champs :");

        static const char *kTypes[] = {"float", "int", "bool", "string"};

        int toRemove = -1;
        for (size_t i = 0; i < m_fields.size(); ++i)
        {
            ImGui::PushID(static_cast<int>(i));

            ImGui::SetNextItemWidth(150.0f);
            ImGui::InputTextWithHint("##fname", "champ...", &m_fields[i].name);
            ImGui::SameLine();

            int typeIdx = 0;
            for (int t = 0; t < 4; ++t)
                if (m_fields[i].type == kTypes[t])
                {
                    typeIdx = t;
                    break;
                }
            ImGui::SetNextItemWidth(90.0f);
            if (ImGui::Combo("##ftype", &typeIdx, kTypes, 4))
                m_fields[i].type = kTypes[typeIdx];

            ImGui::SameLine();
            if (ImGui::SmallButton("X"))
                toRemove = static_cast<int>(i);

            ImGui::PopID();
        }
        if (toRemove >= 0)
            m_fields.erase(m_fields.begin() + toRemove);

        if (ImGui::Button("+ Ajouter un champ"))
            m_fields.push_back({"", "float"});

        ImGui::Separator();
        bool nameValid = !m_name.empty();
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
