#include "visu/App.hpp"

#include "visu/core/Project.hpp"
#include "visu/ui/Panel.hpp"
#include "visu/ui/SystemPanel.hpp"
#include "visu/ui/ComponentPanel.hpp"
#include "visu/ui/MainPanel.hpp"
#include "visu/ui/Modal.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "IconsFontAwesome6.h" // defines ICON_FA_* + ICON_MIN/MAX_FA

#include <GLFW/glfw3.h>
#include <cstdio>
#include <string>
#include <algorithm>

#include <iostream>

static void glfw_error_callback(int error, const char *description)
{
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void buildDefaultLayout(ImGuiID dockspace_id, const ImVec2 &size)
{
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, size);

    // Hierarchy + Inspector sont DANS le panel Main (3 colonnes) -> pas de
    // fenetres separees. Main/System/Component sont de simples onglets freres.
    ImGui::DockBuilderDockWindow("Main", dockspace_id);
    ImGui::DockBuilderDockWindow("System", dockspace_id);
    ImGui::DockBuilderDockWindow("Component", dockspace_id);

    ImGui::DockBuilderFinish(dockspace_id);
}

App::App() = default;

App::~App()
{
    // Nettoyage symétrique de init() — uniquement si la fenêtre a été créée.
    if (m_window)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

void App::flushPanel()
{
    for (auto &p : m_pendingPanels)
    {
        if (p->dock.splitSource != 0)
        {
            ImGuiID newNode = ImGui::DockBuilderSplitNode( // l'enum class dir est deja caller sur la dir de imgui
                p->dock.splitSource, (ImGuiDir)p->dock.splitDir, p->dock.splitRatio, &newNode, nullptr);
            p->dock.dockTarget = newNode;
            p->dock.splitSource = 0;
            ImGui::DockBuilderFinish(m_dockspaceId);
        }
        m_panels.push_back(std::move(p));
    }
    m_pendingPanels.clear();

    m_panels.erase(
        std::remove_if(m_panels.begin(), m_panels.end(),
                       [](const std::unique_ptr<Panel> &p)
                       { return !p->visible && p->removeOnClose; }),
        m_panels.end());
}

bool App::init()
{
    // --- Init GLFW ---
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    // OpenGL 3.3 Core Profile
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1280, 720, "EE-Visu", nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // v-sync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigViewportsNoDecoration = false;

    io.Fonts->AddFontDefault();
    static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig iconConfig;
    iconConfig.MergeMode = true;
    iconConfig.PixelSnapH = true;
    iconConfig.GlyphMinAdvanceX = 13.0f;
    std::string fontPath = std::string(ASSETS_DIR) + "/fonts/fa-solid-900.ttf";
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f, &iconConfig, iconRanges);

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    if (!io.IniFilename || !std::filesystem::exists(io.IniFilename))
        m_needDefaultLayout = true;

    requestPanel(std::make_unique<SystemPanel>());
    requestPanel(std::make_unique<ComponentPanel>());
    requestPanel(std::make_unique<MainPanel>());
    flushPanel();
    return true;
}

void App::openProject(const std::filesystem::path &path)
{
    m_project = std::make_unique<Project>(path);
    m_sceneLoaded = false; // rechargera la scene du nouveau projet
    m_selectedEntity = -1;
}

void App::loadSceneIfNeeded()
{
    if (m_sceneLoaded)
        return;

    m_scene = SceneInfo{};
    m_selectedEntity = -1;
    if (m_project && m_project->isValid())
        if (auto loaded = loadScene(m_project->sceneFile()))
            m_scene = *loaded;
    m_sceneLoaded = true;
}

void App::saveCurrentScene()
{
    if (m_project && m_project->isValid())
        saveScene(m_scene, m_project->sceneFile());
}

void App::requestPanel(std::unique_ptr<Panel> _panel)
{
    m_pendingPanels.push_back(std::move(_panel));
}

void App::openModal(std::unique_ptr<Modal> _modal)
{
    m_currentModal = std::move(_modal);
    m_modalJustOpen = true;
}

void App::drawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Fichier"))
        {
            if (ImGui::MenuItem("Quitter"))
                glfwSetWindowShouldClose(m_window, true);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Fenetre"))
        {
            for (auto &p : m_panels)
                ImGui::MenuItem(p->name(), nullptr, &p->visible);
            ImGui::Separator();
            if (ImGui::MenuItem("Reset layout"))
                m_needDefaultLayout = true;

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void App::drawPanels()
{
    for (auto &p : m_panels)
    {
        if (!p->visible)
            continue;
        if (p->dock.dockTarget != 0)
        {
            ImGui::SetNextWindowDockID(p->dock.dockTarget, ImGuiCond_Once);
            p->dock.dockTarget = 0;
        }

        if (ImGui::Begin(p->name(), &p->visible))
            p->draw(m_project.get());
        ImGui::End();
    }
}

void App::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        // Nouvelle frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        m_dockspaceId = ImGui::DockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_AutoHideTabBar);

        if (m_needDefaultLayout)
        {
            m_needDefaultLayout = false;
            buildDefaultLayout(m_dockspaceId, viewport->Size);
        }
        drawMenuBar();
        drawPanels();

        if (m_currentModal)
        {
            if (m_modalJustOpen)
            {
                ImGui::OpenPopup(m_currentModal->Id());
                m_modalJustOpen = false;
            }
            bool finished = m_currentModal->Draw();
            if (finished)
                m_currentModal.reset();
        }

        flushPanel();

        // Rendu
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.10f, 0.12f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Gestion des fenêtres détachées (viewports)
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(m_window);
    }
}
