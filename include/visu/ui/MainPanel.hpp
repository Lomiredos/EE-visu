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
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
};
