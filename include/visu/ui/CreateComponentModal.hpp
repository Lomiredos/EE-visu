#pragma once
#include "visu/ui/Modal.hpp"

#include <functional>
#include <string>
#include <vector>

// Un champ de composant tel qu'authore : un nom + un type FEUILLE.
struct ComponentFieldDef
{
    std::string name;
    std::string type = "float"; // float | int | bool | string
};

struct ComponentInfoCreation
{
    std::string name;
    std::vector<ComponentFieldDef> fields;
};

// Popup "creer un composant" : nom + liste de champs (nom + type). Ne connait
// pas la generation : elle renvoie un ComponentInfoCreation a un callback, qui
// serialisera (struct .hpp + reflexion). Symetrique de CreateSystemModal.
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
