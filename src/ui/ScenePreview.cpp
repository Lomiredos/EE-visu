#include "visu/ui/ScenePreview.hpp"

#include "visu/render/Renderer.hpp"
#include "visu/systems/RenderSystem.hpp"

#include <cmath>
#include <algorithm>

namespace
{
    void normalize3(float v[3])
    {
        float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (l > 0.0f)
        {
            v[0] /= l;
            v[1] /= l;
            v[2] /= l;
        }
    }

    void forwardFrom(float yaw, float pitch, float out[3])
    {
        float cp = std::cos(pitch), sp = std::sin(pitch);
        float cy = std::cos(yaw), sy = std::sin(yaw);
        out[0] = cp * cy;
        out[1] = sp;
        out[2] = cp * sy;
    }

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

void ScenePreview::ensureCamInit()
{
    if (m_camInit)
        return;
    float d[3] = {-m_camPos[0], -m_camPos[1], -m_camPos[2]}; // viser l'origine au depart
    normalize3(d);
    m_pitch = std::asin(d[1]);
    m_yaw = std::atan2(d[2], d[0]);
    m_camInit = true;
}

unsigned int ScenePreview::render(const SceneInfo &scene, int width, int height, int selected)
{
    if (width <= 0 || height <= 0)
        return 0;

    ensureCamInit();

    ee::render::Camera cam;
    cam.pos = {m_camPos[0], m_camPos[1], m_camPos[2]};
    cam.yaw = m_yaw;
    cam.pitch = m_pitch;

    m_gl.beginScene(cam, width, height);

    // Le "B" : le systeme de rendu traduit la scene en appels de dessin.
    // On lui passe le GLRenderer *comme un IRenderer* : il ne sait pas que
    // c'est de l'OpenGL derriere.
    ee::systems::renderScene(m_gl, scene);

    m_gl.endScene();
    return m_gl.texture();
}

void ScenePreview::addYawPitch(float dYaw, float dPitch)
{
    m_yaw += dYaw;
    m_pitch += dPitch;
    const float lim = 1.55f; // ~89 degres, evite le retournement au zenith
    if (m_pitch > lim)
        m_pitch = lim;
    if (m_pitch < -lim)
        m_pitch = -lim;
}

void ScenePreview::moveLocal(float forward, float right, float up)
{
    float f[3];
    forwardFrom(m_yaw, m_pitch, f);
    float wup[3] = {0.0f, 1.0f, 0.0f};
    float r[3] = {f[1] * wup[2] - f[2] * wup[1],
                  f[2] * wup[0] - f[0] * wup[2],
                  f[0] * wup[1] - f[1] * wup[0]};
    normalize3(r);
    for (int i = 0; i < 3; ++i)
        m_camPos[i] += f[i] * forward + r[i] * right + wup[i] * up;
}

void ScenePreview::dolly(float amount)
{
    float f[3];
    forwardFrom(m_yaw, m_pitch, f);
    for (int i = 0; i < 3; ++i)
        m_camPos[i] += f[i] * amount;
}

void ScenePreview::getViewMatrix(float out[16]) const
{
    m_gl.viewMatrix(out);
}

void ScenePreview::getProjMatrix(float out[16]) const
{
    m_gl.projMatrix(out);
}

int ScenePreview::pick(const SceneInfo &scene, float ndcX, float ndcY, float aspect)
{
    if (!m_camInit)
        return -1; // la camera est initialisee dans render()

    // Base camera + direction du rayon a travers le pixel (ndcX, ndcY).
    float fwd[3];
    forwardFrom(m_yaw, m_pitch, fwd);
    float wup[3] = {0.0f, 1.0f, 0.0f};
    float right[3] = {fwd[1] * wup[2] - fwd[2] * wup[1],
                      fwd[2] * wup[0] - fwd[0] * wup[2],
                      fwd[0] * wup[1] - fwd[1] * wup[0]};
    normalize3(right);
    float camUp[3] = {right[1] * fwd[2] - right[2] * fwd[1],
                      right[2] * fwd[0] - right[0] * fwd[2],
                      right[0] * fwd[1] - right[1] * fwd[0]};

    float t = std::tan(45.0f * 3.14159265f / 180.0f * 0.5f);
    float dir[3];
    for (int i = 0; i < 3; ++i)
        dir[i] = fwd[i] + right[i] * (ndcX * aspect * t) + camUp[i] * (ndcY * t);
    normalize3(dir);

    // Intersection rayon/qqchose, on garde la plus proche.

    int best = -1;
    float bestT = 1e30f; // tres tres gros nombre

    for (int idx = 0; idx < static_cast<int>(scene.entities.size()); ++idx)
    {
        const ComponentInstance *tf = nullptr;
        const ComponentInstance *sp = nullptr;
        const ComponentInstance *rp = nullptr;
        for (const auto &ci : scene.entities[idx].components)
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

        float cx = valueOf(tf->values, "x", 0.0f);
        float cy = valueOf(tf->values, "y", 0.0f);
        float cz = valueOf(tf->values, "z", 0.0f);
        if (sp)
        {
            float R = valueOf(sp->values, "radius", 1.0f);

            float oc[3] = {m_camPos[0] - cx, m_camPos[1] - cy, m_camPos[2] - cz};
            float b = dir[0] * oc[0] + dir[1] * oc[1] + dir[2] * oc[2];
            float c = oc[0] * oc[0] + oc[1] * oc[1] + oc[2] * oc[2] - R * R;
            float disc = b * b - c;

            if (disc > 0.0f)
            {

                float tt = -b - std::sqrt(disc);
                if (tt > 0.0f && tt < bestT)
                {
                    bestT = tt;
                    best = idx;
                }
            }
        }

        if (rp)
        {
            float cw = valueOf(rp->values, "width", 0.0f);
            float ch = valueOf(rp->values, "height", 0.0f);
            float cd = valueOf(rp->values, "depth", 0.0f);

            float t1, t2;

            // x
            float xmin = cx - cw / 2;
            float xmax = cx + cw / 2;

            t1 = (xmin - m_camPos[0]) / dir[0];
            t2 = (xmax - m_camPos[0]) / dir[0];

            float E_x = std::min(t1, t2);
            float S_x = std::max(t1, t2);

            // y
            float ymin = cy - ch / 2;
            float ymax = cy + ch / 2;

            t1 = (ymin - m_camPos[1]) / dir[1];
            t2 = (ymax - m_camPos[1]) / dir[1];

            float E_y = std::min(t1, t2);
            float S_y = std::max(t1, t2);

            // z
            float zmin = cz - cd / 2;
            float zmax = cz + cd / 2;

            t1 = (zmin - m_camPos[2]) / dir[2];
            t2 = (zmax - m_camPos[2]) / dir[2];

            float E_z = std::min(t1, t2);
            float S_z = std::max(t1, t2);

            // on croise

            float tEntree = std::max(E_x, std::max(E_y, E_z));
            float tSortie = std::min(S_x, std::min(S_y, S_z));

            if (tEntree > 0.0f && tEntree < tSortie)
            {
                if (tEntree < bestT)
                {
                    bestT = tEntree;
                    best = idx;
                }
            }
        }
    }
    return best;
}
