#include "EditTextureGUI.h"

#include "../logic/AneditContext.h"
#include <render/scene/Scene.h>
#include <render/asset/AssetCollection.h>

#include <imgui.h>

namespace gui {

EditTextureGUI::EditTextureGUI()
  : IGUI()
{}

EditTextureGUI::~EditTextureGUI()
{}

void EditTextureGUI::immediateDraw(logic::AneditContext* c)
{
  auto id = c->getFirstSelection();

  if (!id || c->selectionType() != logic::AneditContext::SelectionType::Texture) {
    return;
  }

  char name[100];
  name[0] = '\0';

  auto tex = c->assetCollection().getTextureBlocking(id);
  std::string texName = tex._name.empty() ? id.str() : tex._name;

  strcpy_s(name, texName.c_str());

  ImGui::Text("Name");
  if (ImGui::InputText("##texname", name, 100) && id) {
    tex._name = name;
    c->assetCollection().updateTexture(std::move(tex));
  }
  ImGui::Separator();

  float texFactor = 0.75f;
  auto maxRegion = ImGui::GetWindowContentRegionMax();
  ImVec2 texSize{ texFactor * maxRegion.x , texFactor * maxRegion.x };

  auto* texId = c->getImguiTexId(id);
  if (!texId) {
    c->forceLoadTex(id);
  }
  else {
    ImGui::Image((ImTextureID)c->getImguiTexId(id), texSize);
  }
}

}