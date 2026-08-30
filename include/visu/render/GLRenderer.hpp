#pragma once

#include "visu/render/Renderer.hpp"

// ---------------------------------------------------------------------------
// Backend OpenGL du contrat IRenderer.
//
// C'est le "A" extrait de l'ancien ScenePreview : shaders, mesh sphere, FBO,
// matrices, glDrawElements. Il rend dans un framebuffer hors-ecran et expose
// la texture couleur, a afficher via ImGui::Image dans l'editeur.
//
// Il ne connait NI la scene, NI les composants, NI l'ECS : on lui dit
// "dessine une sphere ici", pas "voici une entite".
// ---------------------------------------------------------------------------

namespace ee::render
{
    class GLRenderer : public IRenderer
    {
    public:
        ~GLRenderer() override;

        // --- Contrat IRenderer ---
        void beginScene(const Camera &cam, int width, int height) override;
        void endScene() override;
        void drawSphere(Vec3 pos, float radius, Color col) override;
        void drawBox(Vec3 pos, Vec3 size, Color col) override;

        unsigned int texture() const { return m_colorTex; } // 0 si echec
        void viewMatrix(float out[16]) const;               // column-major, 16 floats
        void projMatrix(float out[16]) const;

    private:
        bool ensureInit();            // charge GL + shader + mesh (une fois)
        void ensureFbo(int w, int h); // (re)cree le FBO si la taille change

        bool m_init = false;
        bool m_failed = false;

        unsigned int m_program = 0;
        unsigned int m_vao = 0, m_vbo = 0, m_ebo = 0; // mesh sphere
        int m_indexCount = 0;
        unsigned int m_cubeVao = 0, m_cubeVbo = 0, m_cubeEbo = 0; // mesh cube
        int m_cubeIndexCount = 0;
        int m_uMVP = -1, m_uModel = -1, m_uColor = -1, m_uGlow = -1;

        unsigned int m_fbo = 0, m_colorTex = 0, m_depthRbo = 0;
        int m_fboW = 0, m_fboH = 0;

        float m_view[16] = {0}; // memorises pour l'editeur (ImGuizmo)
        float m_proj[16] = {0};
        float m_vp[16] = {0}; // proj * view, reutilise a chaque drawSphere
    };
}
