#include "GLTFImporter.h"

#include <util/GLTFLoader.h>

#include <chrono>

namespace logic {

GLTFImporter::GLTFImporter()
{}

GLTFImporter::~GLTFImporter()
{}

void GLTFImporter::startLoad(const std::filesystem::path& path)
{
  _loadingFuture = std::async(std::launch::async, [path] {

    LoadedGLTFData data{};

    util::GLTFLoader::loadFromFile(
      path.string(),
      data._prefabs,
      data._models,
      data._textures,
      data._materials,
      //data._skeletons,
      data._animations);

    return data;
  });
}

bool GLTFImporter::hasResult() const
{
  return _loadingFuture.valid() && 
    _loadingFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

std::unique_ptr<LoadedGLTFData> GLTFImporter::takeResult()
{
  if (hasResult()) {
    return std::make_unique<LoadedGLTFData>(_loadingFuture.get());
  }

  return nullptr;
}

}