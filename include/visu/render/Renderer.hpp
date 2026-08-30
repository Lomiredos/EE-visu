#pragma once

// ---------------------------------------------------------------------------
// Le CONTRAT de rendu.
//
// IRenderer decrit *ce que* le jeu sait montrer (des verbes de haut niveau),
// jamais *comment* c'est dessine. Aucune techno ici : ni gl..., ni raylib.
// Les systemes du jeu ne parlent qu'a cette interface ; un backend concret
// (GLRenderer aujourd'hui, un RaylibRenderer demain) l'implemente.
//
// C'est le renversement de dependance : le gameplay depend de l'abstraction,
// les technos vivent en dessous.
// ---------------------------------------------------------------------------

namespace ee::render
{
    struct Vec3
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
    };

    struct Color
    {
        float r = 1.0f, g = 1.0f, b = 1.0f;
    };

    // Camera decrite en DONNEES, sans techno. Chaque backend en derive ses
    // propres matrices (view/projection).
    struct Camera
    {
        Vec3 pos;
        float yaw = 0.0f;   // rotation autour de Y
        float pitch = 0.0f; // haut/bas
        float fovDeg = 45.0f;
        float nearZ = 0.1f;
        float farZ = 200.0f;
    };

    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        // Ouvre / ferme le rendu d'une scene pour une camera donnee.
        // width/height : taille de la cible en pixels (sert au ratio d'aspect).
        virtual void beginScene(const Camera &cam, int width, int height) = 0;
        virtual void endScene() = 0;

        // Primitives de dessin. On ne met dans le contrat que ce que le jeu
        // utilise reellement aujourd'hui (la sphere). Les prochaines viendront
        // au besoin, au meme niveau d'abstraction :
        //   virtual void drawMesh(MeshHandle mesh, const Transform &t, Color col) = 0;
        //   virtual MeshHandle loadMesh(const std::string &path) = 0;
        // Les ressources circuleront alors via des handles opaques (un id),
        // jamais via un type backend (Model raylib / VAO GL), pour ne pas fuiter.
        virtual void drawSphere(Vec3 pos, float radius, Color col) = 0;
        virtual void drawBox(Vec3 pos, Vec3 size, Color col) = 0;
    };
}
