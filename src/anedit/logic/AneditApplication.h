#pragma once

#include "Application.h"

#include "AneditContext.h"
#include "AneditConfig.h"
#include "GLTFImporter.h"
#include "../gui/IGUI.h"

#include <glm/glm.hpp>

#include <render/VulkanRenderer.h>
#include <render/asset/Model.h>
#include <render/Camera.h>
#include <render/RenderDebugOptions.h>
#include <render/RenderOptions.h>
#include <render/asset/Model.h>
#include <render/asset/AssetCollection.h>
#include <render/scene/Scene.h>
#include <render/scene/ScenePager.h>
#include <render/cinematic/CinematicPlayer.h>
#include <render/animation/AnimationUpdater.h>
#include <physics/PhysicsSystem.h>
#include <terrain/TerrainSystem.h>
#include <behaviour/BehaviourSystem.h>
#include "WindSystem.h"

#include <filesystem>

class AneditApplication : public Application, public logic::AneditContext
{
public:
  AneditApplication(std::string title);
  ~AneditApplication();

  bool init();

  void render() override final;
  void update(double delta) override final;

  void notifyFramebufferResized();

  // AneditContext begin

  render::scene::Scene& scene() override final;
  render::asset::AssetCollection& assetCollection() override final;
  void serializeScene() override final;
  void loadSceneFrom(std::filesystem::path p) override final;
  void setScenePath(std::filesystem::path p) override final;

  void startLoadGLTF(std::filesystem::path p) override final;

  void spawnFromPrefabAtMouse(const util::Uuid& prefab) override final;

  // Does not take children/parents into account!
  render::asset::Prefab prefabFromNode(const util::Uuid& node) override final;

  void* getImguiTexId(util::Uuid& tex) override final;
  void forceLoadTex(const util::Uuid& tex) override final;
  void generateMipMaps(render::asset::Texture& tex) override final;

  // Edit state.
  void playEditor() override final;
  void pauseEditor() override final;
  void stopEditor() override final;
  logic::AneditContext::EditorState getState() override final;

  void createCinematicPlayer(util::Uuid& id) override final;
  void destroyCinematicPlayer(util::Uuid& id) override final;
  void playCinematic(util::Uuid& id) override final;
  void pauseCinematic(util::Uuid& id) override final;
  void stopCinematic(util::Uuid& id) override final;
  double getCinematicTime(util::Uuid& id) override final;
  void setCinematicTime(util::Uuid& id, double time) override final;

  std::vector<util::Uuid>& selection() override final;

  render::Camera& camera() override final;

  virtual glm::vec3 latestWorldPosition() override final { return _latestWorldPosition; }

  // AneditContext end

private:
  void setupGuis();
  void updateConfig();
  void addGltfDataToScene(std::unique_ptr<logic::LoadedGLTFData> data);
  void oldUI();
  void calculateShadowMatrix();
  // The map is <prefab, node>
  util::Uuid instantiate(const render::asset::Prefab& prefab, glm::mat4 parentGlobalTransform, std::unordered_map<util::Uuid, util::Uuid>& instantiatedNodes);
  void updateSkeletons(std::unordered_map<util::Uuid, util::Uuid>& prefabNodeMap);
  void updateCamera(double delta);
  void findCameraNode();
  void registerBehaviours();

  // Keep track of which state we're in. This controls what systems get updated each frame.
  enum class State
  {
    Playing,
    Paused,
    Stopped
  } _state = State::Stopped;

  std::vector<gui::IGUI*> _guis;

  std::unordered_map<util::Uuid, render::cinematic::CinematicPlayer> _cinePlayers;

  std::filesystem::path _scenePath;
  std::filesystem::path _assPath;
  logic::AneditConfig _config;

  logic::GLTFImporter _gltfImporter;

  glm::vec3 _lastCamPos;

  glm::vec3 _sunDir;

  render::RenderDebugOptions _renderDebugOptions;
  render::RenderOptions _renderOptions;
  logic::WindSystem _windSystem;
  glm::vec2 _windDir;

  render::Camera _camera;
  render::Camera _shadowCamera;
  util::Uuid _cameraNode; // Used in play mode, currently we support one main camera.

  render::scene::Scene _scene;
  render::asset::AssetCollection _assColl;
  render::VulkanRenderer _vkRenderer;
  std::future<render::scene::DeserialisedSceneData> _sceneFut;
  render::scene::ScenePager _scenePager;

  render::anim::AnimationUpdater _animUpdater;
  terrain::TerrainSystem _terrainSystem;
  physics::PhysicsSystem _physicsSystem;
  behaviour::BehaviourSystem _behaviourSystem;

  glm::vec3 _latestWorldPosition = glm::vec3(0.0f);

  bool _drawPhysicsDebug = true;

  // Test bake
  bool _baking = false;

  // For force loaded textures (typically so that GUI can draw textures)
  std::mutex _forcedTexMtx;
  std::vector<render::asset::Texture> _loadedForcedTextures;
  std::unordered_map<util::Uuid, int> _pendingForcedTextures;

  void updateForcedTextures();
};