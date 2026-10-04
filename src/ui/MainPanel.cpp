#include "visu/ui/MainPanel.hpp"

#include "visu/App.hpp"
#include "visu/core/Project.hpp"
#include "visu/core/SceneInfo.hpp"
#include "visu/helpers/BuildProject.hpp"
#include "visu/helpers/ComponentGetter.hpp"
#include "visu/helpers/OpenExternal.hpp"
#include "visu/helpers/SceneGen.hpp"
#include "visu/ui/ConfirmModal.hpp"
#include "visu/ui/CreateSceneModal.hpp"

#include "imgui.h"
#include <imgui_stdlib.h> // ImGui::InputText(const char*, std::string*)
#include <memory>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // capture souris + touches pour la navigation camera
#include <ImGuizmo.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <variant>

#include "IconsFontAwesome6.h"
#include "visu/helpers/FilesGetter.hpp"

static float asFloat(const FieldValue &v, float def = 0.0f) {
  if (auto p = std::get_if<float>(&v))
    return *p;
  if (auto p = std::get_if<int>(&v))
    return static_cast<float>(*p);
  if (auto p = std::get_if<bool>(&v))
    return *p ? 1.0f : 0.0f;
  return def;
}

static void drawHierarchy(App &app, Project *project,
                          MainPanel::DeletionState &del,
                          std::string &sceneStatus) {
  SceneInfo &scene = app.sceneData();
  int &selected = app.selectedEntity();
  ImGui::TextUnformatted(app.sceneData().name.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Change Scene")) {
    ImGui::OpenPopup("Scene_Selector");
  }

  if (ImGui::BeginPopup("Scene_Selector")) {
    std::filesystem::path scenesPath = project->scenesDir();
    std::vector<std::string> sceneFiles =
        getFilesInFolderWith(scenesPath, {".hpp"});

    std::vector<std::string> sceneNames;
    for (const std::string &f : sceneFiles)
      sceneNames.push_back(std::filesystem::path(f).stem().string());

    for (const std::string &sceneName : sceneNames) {
      if (ImGui::Selectable(sceneName.c_str())) {
        if (app.sceneDirty()) {
          app.openModal(std::make_unique<ConfirmModal>(
              "Changer de Scene ?",
              "Il y a des modification non sauvegarder dans la scene.",
              std::vector<ModalChoice>{
                  {"Sauvegarder et changer",
                   [&app, sceneName]() {
                     app.saveCurrentScene();
                     app.loadSceneData(sceneName);
                   }},
                  {"Changer sans sauvegarder",
                   [&app, sceneName]() { app.loadSceneData(sceneName); }},
                  {"Annuler"}}));
        } else {
          app.loadSceneData(sceneName);
        }
      }
    }
    ImGui::Separator();
    if (ImGui::Button("+ Nouvelle scene")) {
      std::vector<std::string> parents;
      parents.push_back(kDefaultSceneParent);
      for (const std::string &n : sceneNames)
        parents.push_back(n);

      app.openModal(std::make_unique<CreateSceneModal>(
          std::move(parents), [project, &sceneStatus](SceneInfoCreation _data) {
            scenegen::CreateResult r =
                scenegen::createScene(project->scenesDir(), _data);
            if (r.error.empty()) {
              sceneStatus = "Scene creee.";
              openInVSCode(r.path);
            } else {
              sceneStatus = r.error;
            }
          }));
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  if (!sceneStatus.empty()) {
    ImGui::SameLine();
    ImGui::TextDisabled("%s", sceneStatus.c_str());
  }

  ImGui::Separator();
  if (ImGui::Button("Create Entity")) {
    scene.entities.push_back({});
    selected = static_cast<int>(scene.entities.size()) - 1;
    app.markSceneDirty();
  }
  ImGui::SameLine();
  if (ImGui::Button("Save"))
    app.saveCurrentScene();
  if (app.sceneDirty()) {
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "* non sauvegarde");
  }

  ImGui::Separator();

  for (int i = 0; i < static_cast<int>(scene.entities.size()); ++i) {
    ImGui::SetNextItemAllowOverlap();

    const std::string &nm = scene.entities[i].name;
    std::string display = nm.empty() ? ("Entity " + std::to_string(i)) : nm;
    std::string label = display + "##ent" + std::to_string(i);
    if (ImGui::Selectable(label.c_str(), selected == i))
      selected = i;

    std::string icon = std::string(ICON_FA_TRASH) + "##" + std::to_string(i);

    float iconW = ImGui::CalcTextSize(icon.c_str()).x +
                  ImGui::GetStyle().FramePadding.x * 2.0f;

    ImGui::SameLine(ImGui::GetContentRegionMax().x - iconW);

    if (ImGui::SmallButton(icon.c_str())) {

      del.entityToDelete = i; // memorise QUELLE entite avant d'ouvrir le modal
      del.openDeleteEntityModal = true;
    }
  }
}

// --- Colonne droite : composants + valeurs de l'entite selectionnee ---
static void drawInspector(App &app, Project *project,
                          MainPanel::DeletionState &del) {
  SceneInfo &scene = app.sceneData();
  int selected = app.selectedEntity();

  if (selected < 0 || selected >= static_cast<int>(scene.entities.size())) {
    ImGui::TextDisabled("Selectionne une entite");
    return;
  }

  EntityInfo &ent = scene.entities[selected];
  {
    std::string display =
        ent.name.empty() ? ("Entity " + std::to_string(selected)) : ent.name;
    ImGui::Text("%s", display.c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton("Rename"))
      ImGui::OpenPopup("rename_entity");
  }
  if (ImGui::BeginPopup("rename_entity")) {
    if (ImGui::IsWindowAppearing())
      ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("##rename", &ent.name,
                         ImGuiInputTextFlags_EnterReturnsTrue))
      ImGui::CloseCurrentPopup();
    if (ImGui::IsItemEdited())
      app.markSceneDirty();
    ImGui::SameLine();
    if (ImGui::Button("OK"))
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
  ImGui::Separator();

  for (int c = 0; c < static_cast<int>(ent.components.size()); ++c) {
    ComponentInstance &ci = ent.components[c];
    ImGui::PushID(c);

    ImGui::SetNextItemAllowOverlap();
    bool open = ImGui::CollapsingHeader(ci.name.c_str(),
                                        ImGuiTreeNodeFlags_DefaultOpen);

    const char *icon = ICON_FA_TRASH;

    float iconW =
        ImGui::CalcTextSize(icon).x + ImGui::GetStyle().FramePadding.x * 2.0f;

    ImGui::SameLine(ImGui::GetContentRegionMax().x - iconW);

    if (ImGui::SmallButton(icon)) {
      del.deleteEntity = selected;
      del.deleteComp = c;
      del.openDeleteModal = true;
    }

    if (open) {
      if (ci.values.empty())
        ImGui::TextDisabled("(aucun champ)");
      for (auto &kv : ci.values) {
        // Un widget par type actif du variant.
        // DragFloat/DragInt = scrub facon Unity ; Checkbox pour bool ;
        // InputText (via imgui_stdlib) pour les chaines.
        const char *lbl = kv.first.c_str();
        bool edited = false;
        if (auto p = std::get_if<float>(&kv.second))
          edited = ImGui::DragFloat(lbl, p, 0.05f);
        else if (auto p = std::get_if<int>(&kv.second))
          edited = ImGui::DragInt(lbl, p);
        else if (auto p = std::get_if<bool>(&kv.second))
          edited = ImGui::Checkbox(lbl, p);
        else if (auto p = std::get_if<std::string>(&kv.second))
          edited = ImGui::InputText(lbl, p);
        if (edited)
          app.markSceneDirty();
      }
    }

    ImGui::PopID();
  }

  ImGui::Separator();

  if (ImGui::Button("Add Component"))
    ImGui::OpenPopup("add_comp");

  if (ImGui::BeginPopup("add_comp")) {
    std::map<std::string, std::map<std::string, FieldValue>> catalog =
        getAllComponentDefaults(
            project->componentsCatalog()); // union standard + projet

    bool any = false;
    for (const auto &entry : catalog) {
      const std::string &cname = entry.first;
      bool already = std::any_of(
          ent.components.begin(), ent.components.end(),
          [&](const ComponentInstance &ci) { return ci.name == cname; });
      if (already)
        continue;

      any = true;
      if (ImGui::Selectable(cname.c_str())) {
        ComponentInstance ci;
        ci.name = cname;
        ci.values = entry.second;
        ent.components.push_back(std::move(ci));
        app.markSceneDirty();
      }
    }
    if (!any)
      ImGui::TextDisabled("(rien a ajouter)");

    ImGui::EndPopup();
  }
}

static void drawDeleteConfirm(App &app, MainPanel::DeletionState &del) {
  if (del.openDeleteModal) {
    ImGui::OpenPopup("Supprimer le composant ?");
    del.openDeleteModal = false;
  }

  if (ImGui::BeginPopupModal("Supprimer le composant ?", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    SceneInfo &scene = app.sceneData();
    bool valid = del.deleteEntity >= 0 &&
                 del.deleteEntity < static_cast<int>(scene.entities.size()) &&
                 del.deleteComp >= 0 &&
                 del.deleteComp <
                     static_cast<int>(
                         scene.entities[del.deleteEntity].components.size());

    if (valid)
      ImGui::Text("Supprimer \"%s\" ?", scene.entities[del.deleteEntity]
                                            .components[del.deleteComp]
                                            .name.c_str());
    else
      ImGui::TextUnformatted("Supprimer ce composant ?");

    ImGui::Spacing();

    if (ImGui::Button("Supprimer")) {
      if (valid) {
        auto &comps = scene.entities[del.deleteEntity].components;
        comps.erase(comps.begin() + del.deleteComp);
        app.markSceneDirty();
      }
      del.deleteEntity = del.deleteComp = -1;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Annuler")) {
      del.deleteEntity = del.deleteComp = -1;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

static void drawDeleteEntityConfirm(App &app, MainPanel::DeletionState &del) {
  if (del.openDeleteEntityModal) {
    ImGui::OpenPopup("Supprimer l'entite ?");
    del.openDeleteEntityModal = false;
  }

  if (ImGui::BeginPopupModal("Supprimer l'entite ?", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    SceneInfo &scene = app.sceneData();
    bool valid = del.entityToDelete >= 0 &&
                 del.entityToDelete < static_cast<int>(scene.entities.size());

    if (valid) {
      const std::string &nm = scene.entities[del.entityToDelete].name;
      std::string display =
          nm.empty() ? ("Entity " + std::to_string(del.entityToDelete)) : nm;
      ImGui::Text("Supprimer \"%s\" et tous ses composants ?", display.c_str());
    } else
      ImGui::TextUnformatted("Supprimer cette entite ?");

    ImGui::Spacing();

    if (ImGui::Button("Supprimer")) {
      if (valid) {
        scene.entities.erase(scene.entities.begin() + del.entityToDelete);

        // Rattrape la selection : supprimer decale les indices suivants.
        int &sel = app.selectedEntity();
        if (sel == del.entityToDelete)
          sel = -1; // celle qu'on regardait n'existe plus
        else if (sel > del.entityToDelete)
          --sel; // tout ce qui suivait recule d'un cran
        app.markSceneDirty();
      }
      del.entityToDelete = -1;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Annuler")) {
      del.entityToDelete = -1;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void MainPanel::draw(Project *project) {
  if (!project || !project->isValid()) {
    ImGui::TextDisabled("Aucun projet ouvert");
    return;
  }

  App &app = App::getInstance();
  app.loadSceneIfNeeded();

  const float spacing = ImGui::GetStyle().ItemSpacing.x;
  const float totalW = ImGui::GetContentRegionAvail().x;
  const float leftW = totalW * 0.20f;
  const float rightW = totalW * 0.25f;
  const float centerW = totalW - leftW - rightW - 2.0f * spacing;

  // Colonne gauche : Hierarchy
  ImGui::BeginChild("HierarchyRegion", ImVec2(leftW, 0), true);
  drawHierarchy(app, project, m_del, m_sceneStatus);
  ImGui::EndChild();

  ImGui::SameLine();

  // Colonne centrale : infos projet + Play
  ImGui::BeginChild("CenterRegion", ImVec2(centerW, 0), true);
  ImGui::Text("Projet ouvert : %s", project->name().c_str());
  ImGui::Separator();
  {
    // Build : compile le projet, montre les erreurs -> boucle sans quitter.
    if (ImGui::Button("Build")) {
      projectbuild::BuildResult r = projectbuild::build(project->root());
      if (r.ok)
        m_buildStatus = "Build OK.";
      else {
        m_buildStatus = "Echec du build.";
        m_buildLog = r.log;
        m_openBuildErrorPopup = true;
      }
    }
    ImGui::SameLine();

    std::filesystem::path exe = project->executablePath();
    bool exists = std::filesystem::exists(exe);
    if (!exists)
      ImGui::BeginDisabled();
    if (ImGui::Button("Play")) {
      app.saveCurrentScene(); // sauve les edits avant de lancer -> impact
                              // immediat
      launchProgram(exe);
    }
    if (!exists) {
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::TextDisabled("(exe introuvable - build le projet)");
    }

    if (!m_buildStatus.empty())
      ImGui::TextDisabled("%s", m_buildStatus.c_str());

    // Popup d'erreur (bloquant) : sortie du compilo.
    if (m_openBuildErrorPopup) {
      ImGui::OpenPopup("Erreur de build");
      m_openBuildErrorPopup = false;
    }
    if (ImGui::BeginPopupModal("Erreur de build", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::TextUnformatted("Le build a echoue. Sortie :");
      ImGui::BeginChild("buildlog", ImVec2(720, 320), true,
                        ImGuiWindowFlags_HorizontalScrollbar);
      ImGui::TextUnformatted(m_buildLog.c_str());
      ImGui::EndChild();
      if (ImGui::Button("Fermer"))
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  ImGui::Separator();

  // Preview 3D : rendu offscreen des spheres de la scene, affiche via
  // ImGui::Image.
  {
    float hintH = ImGui::GetTextLineHeightWithSpacing();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int pw = static_cast<int>(avail.x);
    int ph = static_cast<int>(avail.y - hintH); // reserve une ligne pour l'aide
    unsigned int tex =
        (pw > 0 && ph > 0)
            ? m_preview.render(app.sceneData(), pw, ph, app.selectedEntity())
            : 0;
    if (tex != 0)
      ImGui::Image((ImTextureID)(intptr_t)tex,
                   ImVec2(static_cast<float>(pw), static_cast<float>(ph)),
                   ImVec2(0, 1),
                   ImVec2(1, 0)); // flip V (FBO origine bas-gauche)

    ImVec2 imgMin = ImGui::GetItemRectMin();
    GLFWwindow *win = glfwGetCurrentContext();

    // --- Gizmo de translation (ImGuizmo) sur l'entite selectionnee ---
    bool gizmoActive = false;
    ImGuizmo::BeginFrame();
    {
      int sel = app.selectedEntity();
      SceneInfo &sc = app.sceneData();
      if (tex != 0 && !m_navMode && sel >= 0 &&
          sel < static_cast<int>(sc.entities.size())) {
        ComponentInstance *tf = nullptr;
        for (auto &ci : sc.entities[sel].components)
          if (ci.name == "TransformComponent") {
            tf = &ci;
            break;
          }
        if (tf) {
          auto gv = [&](const char *k) -> float {
            auto it = tf->values.find(k);
            return it != tf->values.end() ? asFloat(it->second) : 0.0f;
          };

          float vmat[16], pmat[16];
          m_preview.getViewMatrix(vmat);
          m_preview.getProjMatrix(pmat);

          // Compose la matrice modele depuis position, rotation ET echelle
          // courantes : le gizmo part de l'etat reel de l'entite.
          float scx = gv("scaleX"), scy = gv("scaleY"), scz = gv("scaleZ");
          if (scx <= 0.0f) // entites anterieures aux champs scale
            scx = 1.0f;
          if (scy <= 0.0f)
            scy = 1.0f;
          if (scz <= 0.0f)
            scz = 1.0f;
          float translation[3] = {gv("x"), gv("y"), gv("z")};
          float rotation[3] = {gv("rotX"), gv("rotY"), gv("rotZ")};
          float scale[3] = {scx, scy, scz};
          float model[16];
          ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale,
                                                  model);

          ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
          if (m_gizmoMode == 1)
            op = ImGuizmo::ROTATE;
          else if (m_gizmoMode == 2)
            op = ImGuizmo::SCALE;
          ImGuizmo::SetOrthographic(false);
          ImGuizmo::SetDrawlist();
          ImGuizmo::SetRect(imgMin.x, imgMin.y, static_cast<float>(pw),
                            static_cast<float>(ph));
          ImGuizmo::Manipulate(vmat, pmat, op, ImGuizmo::WORLD, model);

          if (ImGuizmo::IsUsing()) {
            // Redecompose : position + rotation + echelle uniforme.
            ImGuizmo::DecomposeMatrixToComponents(model, translation, rotation,
                                                  scale);
            tf->values["x"] = translation[0];
            tf->values["y"] = translation[1];
            tf->values["z"] = translation[2];
            tf->values["rotX"] = rotation[0];
            tf->values["rotY"] = rotation[1];
            tf->values["rotZ"] = rotation[2];
            tf->values["scaleX"] = scale[0];
            tf->values["scaleY"] = scale[1];
            tf->values["scaleZ"] = scale[2];
            app.markSceneDirty();
          }
          gizmoActive = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
        }
      }
    }

    // Double-clic sur la preview -> entre en mode navigation (curseur capture).
    if (tex != 0 && !gizmoActive && ImGui::IsItemHovered() &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      m_navMode = true;
      m_navJustEntered = true;
      if (win)
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // Clic simple sur la preview (hors navigation, hors gizmo) -> selectionne.
    if (tex != 0 && !m_navMode && !gizmoActive && ImGui::IsItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      ImVec2 rmin = ImGui::GetItemRectMin();
      ImVec2 rmax = ImGui::GetItemRectMax();
      ImVec2 mp = ImGui::GetMousePos();
      float w = rmax.x - rmin.x, h = rmax.y - rmin.y;
      if (w > 0.0f && h > 0.0f) {
        float u = (mp.x - rmin.x) / w;
        float vv = (mp.y - rmin.y) / h;
        float ndcX = 2.0f * u - 1.0f;
        float ndcY = 1.0f - 2.0f * vv; // ecran: haut = +1
        int hit = m_preview.pick(app.sceneData(), ndcX, ndcY, w / h);
        if (hit >= 0)
          app.selectedEntity() = hit;
      }
    }

    if (m_navMode) {
      ImGuiIO &io = ImGui::GetIO();
      float dt = io.DeltaTime > 0.0f ? io.DeltaTime : 1.0f / 60.0f;

      // Rotation : delta souris lu directement via GLFW (fiable curseur
      // capture).
      if (win) {
        double mx, my;
        glfwGetCursorPos(win, &mx, &my);
        if (m_navJustEntered) {
          m_lastMouseX = mx;
          m_lastMouseY = my;
          m_navJustEntered = false;
        }
        float dx = static_cast<float>(mx - m_lastMouseX);
        float dy = static_cast<float>(my - m_lastMouseY);
        m_lastMouseX = mx;
        m_lastMouseY = my;
        m_preview.addYawPitch(dx * 0.0025f, -dy * 0.0025f);
      }

      // Deplacement ZQSD (W/Z et A/Q pour AZERTY/QWERTY), Espace/Shift =
      // haut/bas.
      float speed = 5.0f * dt, f = 0.0f, r = 0.0f, u = 0.0f;
      if (ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_Z))
        f += 1.0f;
      if (ImGui::IsKeyDown(ImGuiKey_S))
        f -= 1.0f;
      if (ImGui::IsKeyDown(ImGuiKey_D))
        r += 1.0f;
      if (ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_Q))
        r -= 1.0f;
      if (ImGui::IsKeyDown(ImGuiKey_Space))
        u += 1.0f;
      if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
        u -= 1.0f;
      m_preview.moveLocal(f * speed, r * speed, u * speed);

      // Zoom molette.
      if (io.MouseWheel != 0.0f)
        m_preview.dolly(io.MouseWheel * 0.5f);

      // Echap -> sortir.
      if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_navMode = false;
        if (win)
          glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }

      ImGui::TextDisabled(
          "ZQSD deplacer | souris tourner | molette zoom | Echap sortir");
    } else {
      if (ImGui::RadioButton("Deplacer", m_gizmoMode == 0))
        m_gizmoMode = 0;
      ImGui::SameLine();
      if (ImGui::RadioButton("Tourner", m_gizmoMode == 1))
        m_gizmoMode = 1;
      ImGui::SameLine();
      if (ImGui::RadioButton("Redim.", m_gizmoMode == 2))
        m_gizmoMode = 2;
      ImGui::SameLine();
      ImGui::TextDisabled("| Double-clic pour naviguer");
    }
  }
  ImGui::EndChild();

  ImGui::SameLine();

  // Colonne droite : Inspector
  ImGui::BeginChild("InspectorRegion", ImVec2(rightW, 0), true);
  drawInspector(app, project, m_del);
  ImGui::EndChild();

  // Modal de confirmation au niveau de Main (hors des children).
  drawDeleteConfirm(app, m_del);
  drawDeleteEntityConfirm(app, m_del);
}
