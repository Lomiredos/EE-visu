#pragma once

#include "visu/ui/Panel.hpp"

#include <string>

class SystemPanel : public Panel
{

public:

    const char *name() const override { return "System"; }
    void draw(Project *project) override;

private:
    std::string m_status; // resultat de la derniere creation de systeme
};
