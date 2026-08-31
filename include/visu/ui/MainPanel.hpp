#pragma once

#include "visu/ui/Panel.hpp"
#include "visu/ui/ScenePreview.hpp"

class MainPanel : public Panel
{
public:
    const char *name() const override { return "Main"; }
    void draw(Project *project) override;

private:
    ScenePreview m_preview;

    bool m_navMode = false;
    bool m_navJustEntered = false;
    int m_gizmoMode = 0; // gizmo : 0 = translation, 1 = rotation, 2 = echelle
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
};
