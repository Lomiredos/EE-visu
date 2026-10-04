#pragma once

#include "visu/ui/Panel.hpp"
#include "visu/ui/ScenePreview.hpp"

#include <string>

class MainPanel : public Panel
{
public:
    const char *name() const override { return "Main"; }
    void draw(Project *project) override;

    // Suppressions en attente de confirmation, partagees entre Hierarchy,
    // Inspector et les modals. Membre (plus de static de fichier) : l'etat est
    // porte par l'instance du panneau.
    struct DeletionState
    {
        int deleteEntity = -1;      // composant : entite ciblee
        int deleteComp = -1;        // composant : index du composant
        bool openDeleteModal = false;
        int entityToDelete = -1;    // entite ciblee
        bool openDeleteEntityModal = false;
    };

private:
    ScenePreview m_preview;
    DeletionState m_del;

    // Build du projet (bouton Build) : statut + log + popup d'erreur.
    std::string m_buildStatus;
    std::string m_buildLog;
    bool m_openBuildErrorPopup = false;

    // Creation de scene (bouton "+ Nouvelle scene") : statut de la derniere
    // generation (vide si succes et rien a signaler).
    std::string m_sceneStatus;

    bool m_navMode = false;
    bool m_navJustEntered = false;
    int m_gizmoMode = 0; // gizmo : 0 = translation, 1 = rotation, 2 = echelle
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
};
