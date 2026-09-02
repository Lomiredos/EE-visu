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
    bool m_modalJustOpen = false;
    bool m_needDefaultLayout = false;
    unsigned int m_dockspaceId = 0;

    // Etat de scene partage entre Hierarchy et Inspector.
    SceneInfo m_scene;
    int m_selectedEntity = -1;
    bool m_sceneLoaded = false;
    bool m_sceneDirty = false; // edits non sauvegardes

    // Flux de fermeture : confirmer si des edits ne sont pas sauvegardes.
    bool m_showQuitModal = false;
    bool m_quitConfirmed = false;

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

    // Suivi des modifications non sauvegardees.
    void markSceneDirty() { m_sceneDirty = true; }
    bool sceneDirty() const { return m_sceneDirty; }

    // Demande de fermeture : ouvre la confirmation si des edits sont en attente,
    // sinon ferme la fenetre. Appele par le menu Quitter et le bouton X.
    void requestQuit();

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
    void drawQuitModal();
};
