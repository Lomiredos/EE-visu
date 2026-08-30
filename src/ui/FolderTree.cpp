#include "visu/ui/FolderTree.hpp"
#include "imgui.h"

namespace fs = std::filesystem;

fs::path drawFolderTree(const fs::path &_dir,
                        std::function<void(const std::filesystem::path &)> _onDoubleClick,
                        const std::vector<std::string> &_showExtensions)
{
    fs::path clicked;
    for (const auto &entry : fs::directory_iterator(_dir))
    {
        if (entry.is_directory())
        {
            if (ImGui::TreeNode(entry.path().filename().string().c_str()))
            {
                // On remonte le clic venant des sous-dossiers.
                fs::path sub = drawFolderTree(entry.path(), _onDoubleClick, _showExtensions);
                if (!sub.empty())
                    clicked = sub;
                ImGui::TreePop();
            }
        }
        else
        {
            bool valide = false;
            if (_showExtensions.empty() == false)
            {
                for (const std::string &ext : _showExtensions)
                {
                    if (entry.path().extension() == ext)
                    {
                        valide = true;
                        break;
                    }
                }
            }
            if (valide == false)
                continue;

            ImGui::TreeNodeEx(entry.path().filename().string().c_str(),
                              ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            if (ImGui::IsItemClicked())
            {
                clicked = entry.path();
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                _onDoubleClick(entry.path());
        }
    }
    return clicked;
}
