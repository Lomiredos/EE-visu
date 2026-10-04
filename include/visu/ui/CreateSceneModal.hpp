#pragma once
#include "visu/ui/Modal.hpp"

#include <functional>
#include <string>
#include <vector>

inline const char *const kDefaultSceneParent = "ee::Scene";

struct SceneInfoCreation {
  std::string name;
  std::string parent;
  std::string heritage;
};

class CreateSceneModal : public Modal {
private:
  std::string m_name;
  std::vector<std::string> m_parents;
  int m_selectedParentIdx = 0;
  int m_selectedHeritageIdx = 0;

  std::function<void(SceneInfoCreation)> m_onClose;

  SceneInfoCreation buildResult();

public:
  CreateSceneModal(std::vector<std::string> _parents,
                   std::function<void(SceneInfoCreation)> _cb);
  const char *Id() const override { return "CreateSceneModal"; }
  bool Draw() override;
};
