#pragma once
#include "visu/ui/Modal.hpp"

#include <functional>
#include <string>
#include <vector>

struct ComponentFieldDef
{
    std::string name;
    std::string type = "float";
};

struct ComponentInfoCreation
{
    std::string name;
    std::vector<ComponentFieldDef> fields;
};

class CreateComponentModal : public Modal
{
private:
    std::string m_name;
    std::vector<ComponentFieldDef> m_fields;

    std::function<void(ComponentInfoCreation)> m_onClose;

    ComponentInfoCreation buildResult();

public:
    CreateComponentModal(std::function<void(ComponentInfoCreation)> _cb);
    const char *Id() const override { return "CreateComponentModal"; }
    bool Draw() override;
};
