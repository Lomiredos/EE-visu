#pragma once

#include "visu/ui/Panel.hpp"
#include "visu/core/SystemInfo.hpp"

#include "TextEditor.h"

#include <filesystem>
#include <optional>
namespace fs = std::filesystem;

class ShowCodePanel : public Panel
{
    std::string m_title;
    std::string m_idSuffix;
    fs::path m_filePath;
    TextEditor m_editor;
    unsigned int m_dockNode = 0;
    std::optional<SystemInfo> m_info;

public:
    ShowCodePanel(fs::path _path);

    const char *name() const override { return m_title.c_str(); }
    void draw(Project *project) override;
    void changePath(const fs::path &_path);

    unsigned int dockNode() const { return m_dockNode; }
};
