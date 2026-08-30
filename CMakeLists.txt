cmake_minimum_required(VERSION 3.20)
project(eliott-visu VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(eliott-visu
    src/main.cpp
    src/App.cpp
    src/core/Project.cpp
    src/ui/FolderTree.cpp
    src/ui/SystemPanel.cpp
    src/ui/ComponentPanel.cpp
    src/ui/MainPanel.cpp
    src/ui/ShowCodePanel.cpp
    src/ui/FileReader.cpp
    src/helpers/OpenExternal.cpp
    src/ui/Modal.cpp
    src/ui/CreateSystemModal.cpp
    src/helpers/ComponentGetter.cpp
    src/core/SystemInfo.cpp
    src/core/SceneInfo.cpp
    src/ui/ScenePreview.cpp
    src/render/GLRenderer.cpp
    src/systems/RenderSystem.cpp
    extern/ImGuiColorTextEdit/TextEditor.cpp
    extern/ImGuiColorTextEdit/LanguageDefinitions.cpp
)

target_include_directories(eliott-visu PUBLIC
    include/
    extern/ImGuiColorTextEdit/
    extern/fonts/
)

target_compile_definitions(eliott-visu PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)

target_link_libraries(eliott-visu PRIVATE
    imgui::imgui
    glfw
    OpenGL::GL
    glad::glad
    imguizmo::imguizmo
)
