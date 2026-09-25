cmake_minimum_required(VERSION 3.20)
project(eliott-visu VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
)

target_link_libraries(imgui PUBLIC glfw)
add_library(imgui::imgui ALIAS imgui)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imguizmo)

target_link_libraries(imguizmo PUBLIC imgui)

find_package(OpenGL REQUIRED)

add_library(glad STATIC
    extern/glad/src/glad.c
)

target_include_directories(glad PUBLIC extern/glad/include)
target_link_libraries(glad PUBLIC OpenGL::GL)
add_library(glad::glad ALIAS glad)

add_subdirectory(extern/eliott-engine-3d "${CMAKE_BINARY_DIR}/ee-core")

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
    src/helpers/SystemGen.cpp
    src/ui/Modal.cpp
    src/ui/CreateSystemModal.cpp
    src/ui/CreateComponentModal.cpp
    src/helpers/ComponentGetter.cpp
    src/helpers/ComponentGen.cpp
    src/helpers/GenerateCatalog.cpp
    src/helpers/BuildProject.cpp
    src/ui/ScenePreview.cpp
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

target_link_libraries(eliott-visu PRIVATE
    ee-core          # coeur moteur : amene aussi glad, glfw, OpenGL, nlohmann_json
    imgui::imgui
    imguizmo::imguizmo
)
