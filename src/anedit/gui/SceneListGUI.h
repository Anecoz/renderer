#pragma once

#include "IGUI.h"

#include <util/Uuid.h>

namespace gui {

class SceneListGUI : public IGUI
{
public:
  SceneListGUI();
  ~SceneListGUI();

  void immediateDraw(logic::AneditContext* c) override final;

private:
  void renderNodeTree(util::Uuid& node, logic::AneditContext* c);

  void saveSceneAsClicked(logic::AneditContext* c);
  void saveSceneClicked(logic::AneditContext* c);
  void loadSceneClicked(logic::AneditContext* c);

  void addEmptyClicked(logic::AneditContext* c);
  void addLightClicked(logic::AneditContext* c);
  void addNodeClicked(logic::AneditContext* c);
  void addCinematicClicked(logic::AneditContext* c);

  void deleteNodeClicked(logic::AneditContext* c, util::Uuid& node);
  void nodeDraggedToNode(logic::AneditContext* c, util::Uuid& draggedNode, util::Uuid& droppedOnNode);
};

}