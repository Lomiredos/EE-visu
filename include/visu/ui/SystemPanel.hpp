#pragma once

#include "visu/ui/Panel.hpp"

class SystemPanel : public Panel
{

public:

    const char *name() const override { return "System"; }
    void draw(Project *project) override;
};
