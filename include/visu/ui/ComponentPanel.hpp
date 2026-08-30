#pragma once

#include "visu/ui/Panel.hpp"

class ComponentPanel : public Panel
{
public:
    const char *name() const override { return "Component"; }
    void draw(Project *project) override;
};
