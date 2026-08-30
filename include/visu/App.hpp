#pragma once

#include <filesystem>
#include "visu/ui/Panel.hpp"
#include "visu/core/SceneInfo.hpp"
#include <memory>
#include <vector>

struct GLFWwindow;
class Project;
class Modal;

class App
{
private:
    GLFWwindow *m_window = nullptr;
    std::unique_ptr<Project> m_project;
    std::vector<std::unique_ptr<Panel>> m_panels;
    std::vector<std::unique_ptr<Panel>> m_pendingPanels;
    std::unique_ptr<Modal> m_currentModal;
    bool m_modalJustOpen;
    bool m_needDefaultLayout = false;
    unsigned int m_dockspaceId;

    // Etat de scene partage entre Hierarchy et Inspector.
    SceneInfo m_scene;
    int m_selectedEntity = -1;
    bool m_sceneLoaded = false;

    App();
    ~App();

public:
    static App &getInstance()
    {
        static App instance = App();
        return instance;
    }

    bool init();
    void run();
    void openProject(const std::filesystem::path &path);
    void requestPanel(std::unique_ptr<Panel> _panel);
    void openModal(std::unique_ptr<Modal> _modal);

    // Scene partagee (Hierarchy / Inspector).
    SceneInfo &sceneData() { return m_scene; }
    int &selectedEntity() { return m_selectedEntity; }
    void loadSceneIfNeeded(); // charge depuis le projet courant une seule fois
    void saveCurrentScene();  // ecrit vers le projet courant

    template <typename T>
    T *getPanel(bool unlockedOnly = false)
    {
        for (auto &p : m_panels)
        {
            if (!p->visible)
                continue;
            if (auto *found = dynamic_cast<T *>(p.get()))
                if (!unlockedOnly || !found->isLocked())
                    return found;
        }
        return nullptr;
    }

private:
    void flushPanel();
    void drawMenuBar();
    void drawPanels();
};
