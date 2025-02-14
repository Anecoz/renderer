#pragma once

#include "RenderPass.h"

namespace render {

class UpdateBlasPass : public RenderPass
{
public:
  UpdateBlasPass();
  ~UpdateBlasPass();

  // Register how the render pass will actually render
  void registerToGraph(FrameGraphBuilder&, RenderContext* rc) override final;
};

}