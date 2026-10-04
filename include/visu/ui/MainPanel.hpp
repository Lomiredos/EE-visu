#pragma once

#include "visu/ui/Panel.hpp"
#include "visu/ui/ScenePreview.hpp"

#include <string>

class MainPanel : public Panel {
public:
  const char *name() const override { return "Main"; }
  void draw(Project *project) override;

  struct DeletionState {
    int deleteEntity = -1;
    int deleteComp = -1;
    bool openDeleteModal = false;
    int entityToDelete = -1;
    bool openDeleteEntityModal = false;
  };

  struct NavState {
    bool navMode = false;
    bool navJustEntered = false;
    int gizmoMode = 0;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
  };

private:
  ScenePreview m_preview;
  DeletionState m_del;
  NavState m_nav;

  std::string m_buildStatus;
  std::string m_buildLog;
  bool m_openBuildErrorPopup = false;

  std::string m_sceneStatus;
};
