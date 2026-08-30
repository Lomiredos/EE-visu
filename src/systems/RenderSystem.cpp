#include "visu/systems/RenderSystem.hpp"

#include "visu/render/Renderer.hpp"

#include <variant>

namespace
{
    // Lecture d'un champ multi-type en float (bool/int/float -> float).
    float valueOf(const std::map<std::string, FieldValue> &vals, const char *key, float def)
    {
        auto it = vals.find(key);
        if (it == vals.end())
            return def;
        if (auto p = std::get_if<float>(&it->second))
            return *p;
        if (auto p = std::get_if<int>(&it->second))
            return static_cast<float>(*p);
        if (auto p = std::get_if<bool>(&it->second))
            return *p ? 1.0f : 0.0f;
        return def; // string -> non numerique
    }
}

namespace ee::systems
{
    void renderScene(ee::render::IRenderer &r, const SceneInfo &scene)
    {
        for (const auto &ent : scene.entities)
        {
            const ComponentInstance *tf = nullptr;
            const ComponentInstance *sp = nullptr;
            const ComponentInstance *rp = nullptr;
            for (const auto &ci : ent.components)
            {
                if (ci.name == "TransformComponent")
                    tf = &ci;
                else if (ci.name == "SphereComponent")
                    sp = &ci;
                else if (ci.name == "RectComponent")
                    rp = &ci;
            }
            if (!tf)
                continue;

            ee::render::Vec3 pos{valueOf(tf->values, "x", 0.0f),
                                 valueOf(tf->values, "y", 0.0f),
                                 valueOf(tf->values, "z", 0.0f)};

            if (sp)
            {
                float radius = valueOf(sp->values, "radius", 1.0f);
                r.drawSphere(pos, radius, ee::render::Color{0.85f, 0.20f, 0.20f});
            }

            if (rp)
            {
                ee::render::Vec3 size{valueOf(rp->values, "width", 0.0f),
                                      valueOf(rp->values, "height", 0.0f),
                                      valueOf(rp->values, "depth", 0.0f)};
                r.drawBox(pos, size, ee::render::Color{0.85f, 0.20f, 0.20f});
            }
        }
    }
}
