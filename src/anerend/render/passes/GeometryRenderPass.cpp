#include "GeometryRenderPass.h"

#include "../RenderResource.h"
#include "../ImageHelpers.h"
#include "../FrameGraphBuilder.h"
#include "../RenderResourceVault.h"
#include "../BufferHelpers.h"
#include "../AllocatedBuffer.h"

#include <array>
#include <memory>

namespace render {

namespace {

void initDepthImage(RenderContext* renderContext, VkImage& image)
{
  // Create a staging buffer on CPU side first
  AllocatedBuffer stagingBuffer;
  std::size_t dataSize = renderContext->swapChainExtent().height * renderContext->swapChainExtent().width * sizeof(float);

  auto allocator = renderContext->vmaAllocator();
  bufferutil::createBuffer(allocator, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

  auto cmdBuffer = renderContext->beginSingleTimeCommands();

  // Fill with 0's
  VkDeviceSize bufOffset{};
  vkCmdFillBuffer(cmdBuffer, stagingBuffer._buffer, bufOffset, dataSize, 0);

  // Barrier after filling
  VkBufferMemoryBarrier memBarr{};
  memBarr.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  memBarr.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  memBarr.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  memBarr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  memBarr.buffer = stagingBuffer._buffer;
  memBarr.offset = 0;
  memBarr.size = VK_WHOLE_SIZE;

  vkCmdPipelineBarrier(
    cmdBuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    0, 0, nullptr,
    1, &memBarr,
    0, nullptr);

  // Transition image to dst optimal
  imageutil::transitionImageLayout(
    cmdBuffer,
    image,
    VK_FORMAT_D32_SFLOAT,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkExtent3D extent{};
  extent.depth = 1;
  extent.height = 4;
  extent.width = 4;

  VkOffset3D offset{};

  VkBufferImageCopy imCopy{};
  imCopy.imageExtent = extent;
  imCopy.imageOffset = offset;
  imCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  imCopy.imageSubresource.layerCount = 1;

  vkCmdCopyBufferToImage(
    cmdBuffer,
    stagingBuffer._buffer,
    image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &imCopy);

  // Transition to shader read
  imageutil::transitionImageLayout(
    cmdBuffer,
    image,
    VK_FORMAT_D32_SFLOAT,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

  renderContext->endSingleTimeCommands(cmdBuffer);

  vmaDestroyBuffer(allocator, stagingBuffer._buffer, stagingBuffer._allocation);
}

}

GeometryRenderPass::GeometryRenderPass()
  : RenderPass()
{}

GeometryRenderPass::~GeometryRenderPass()
{}

void GeometryRenderPass::registerToGraph(FrameGraphBuilder& fgb, RenderContext* rc)
{
  RenderPassRegisterInfo info{};
  info._name = "Geometry";
  info._group = "Geometry";

  std::vector<ResourceUsage> resourceUsages;
  {
    ResourceUsage usage{};
    usage._resourceName = "Geometry0Image";
    usage._access.set((std::size_t)Access::Write);
    usage._stage.set((std::size_t)Stage::Fragment);

    ImageInitialCreateInfo createInfo{};
    createInfo._initialHeight = rc->swapChainExtent().height;
    createInfo._initialWidth = rc->swapChainExtent().width;
    createInfo._intialFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

    usage._imageCreateInfo = createInfo;
    usage._type = Type::ColorAttachment;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "Geometry1Image";
    usage._access.set((std::size_t)Access::Write);
    usage._stage.set((std::size_t)Stage::Fragment);

    ImageInitialCreateInfo createInfo{};
    createInfo._initialHeight = rc->swapChainExtent().height;
    createInfo._initialWidth = rc->swapChainExtent().width;
    createInfo._intialFormat = VK_FORMAT_R8G8B8A8_UNORM;

    usage._imageCreateInfo = createInfo;

    usage._type = Type::ColorAttachment;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "Geometry2Image";
    usage._access.set((std::size_t)Access::Write);
    usage._stage.set((std::size_t)Stage::Fragment);

    ImageInitialCreateInfo createInfo{};
    createInfo._initialHeight = rc->swapChainExtent().height;
    createInfo._initialWidth = rc->swapChainExtent().width;
    createInfo._intialFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

    usage._imageCreateInfo = createInfo;

    usage._type = Type::ColorAttachment;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "GeometryDepthImage";
    usage._access.set((std::size_t)Access::Write);
    usage._stage.set((std::size_t)Stage::Fragment);

    ImageInitialCreateInfo createInfo{};
    createInfo._initialHeight = rc->swapChainExtent().height;
    createInfo._initialWidth = rc->swapChainExtent().width;
    createInfo._intialFormat = VK_FORMAT_D32_SFLOAT;
    createInfo._initialDataCb = initDepthImage;
    createInfo._flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    usage._imageCreateInfo = createInfo;

    usage._type = Type::DepthAttachment;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "CullTransBuf";
    usage._access.set((std::size_t)Access::Read);
    usage._stage.set((std::size_t)Stage::Vertex);
    usage._type = Type::SSBO;
    usage._multiBuffered = true;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "CompactedCullDrawBuf";
    usage._access.set((std::size_t)Access::Read);
    usage._stage.set((std::size_t)Stage::IndirectDraw);
    usage._type = Type::SSBO;
    usage._multiBuffered = true;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "RenderableBuffer";
    usage._access.set((std::size_t)Access::Read);
    usage._stage.set((std::size_t)Stage::Vertex);
    usage._type = Type::SSBO;
    usage._multiBuffered = true;
    usage._ownedByEngine = true;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "GigaVtxBuffer";
    usage._access.set((std::size_t)Access::Write);
    usage._stage.set((std::size_t)Stage::Vertex);
    usage._type = Type::SSBO;
    usage._ownedByEngine = true;
    usage._multiBuffered = true;
    resourceUsages.emplace_back(std::move(usage));
  }
  {
    ResourceUsage usage{};
    usage._resourceName = "CompactDrawCountBuf";
    usage._access.set((std::size_t)Access::Read);
    usage._stage.set((std::size_t)Stage::IndirectDraw);
    usage._type = Type::SSBO;
    usage._multiBuffered = true;
    resourceUsages.emplace_back(std::move(usage));
  }

  info._resourceUsages = std::move(resourceUsages);

  GraphicsPipelineCreateParams param{};
  param.device = rc->device();
  param.vertShader = "standard_vert.spv";
  param.fragShader = "standard_frag.spv";
  param.uvLoc = 3;
  param.tangentLoc = 4;
  param.jointLoc = 5;
  param.jointWeightLoc = 6;
  param.colorAttachmentCount = 3;
  param.colorFormats = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R16G16B16A16_SFLOAT };
  info._graphicsParams = param;

  fgb.registerRenderPass(std::move(info));

  fgb.registerRenderPassExe("Geometry",
    [this](RenderExeParams exeParams) {

      // Dynamic rendering begin
      std::array<VkClearValue, 2> clearValues{};
      std::array<VkRenderingAttachmentInfoKHR, 3> colInfo{};
      clearValues[0].color = { {0.5f, 0.5f, 0.5f, 1.0f} };
      clearValues[1].depthStencil = { 1.0f, 0 };
      VkRenderingAttachmentInfoKHR color0AttachmentInfo{};
      color0AttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
      color0AttachmentInfo.imageView = exeParams.colorAttachmentViews[0];
      color0AttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      color0AttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      color0AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color0AttachmentInfo.clearValue = clearValues[0];

      VkRenderingAttachmentInfoKHR color1AttachmentInfo{};
      color1AttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
      color1AttachmentInfo.imageView = exeParams.colorAttachmentViews[1];
      color1AttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      color1AttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      color1AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color1AttachmentInfo.clearValue = clearValues[0];

      VkRenderingAttachmentInfoKHR color2AttachmentInfo{};
      color2AttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
      color2AttachmentInfo.imageView = exeParams.colorAttachmentViews[2];
      color2AttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      color2AttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      color2AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color2AttachmentInfo.clearValue = clearValues[0];

      colInfo[0] = color0AttachmentInfo;
      colInfo[1] = color1AttachmentInfo;
      colInfo[2] = color2AttachmentInfo;

      VkRenderingAttachmentInfoKHR depthAttachmentInfo{};
      depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
      depthAttachmentInfo.imageView = exeParams.depthAttachmentViews[0];
      depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
      depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      depthAttachmentInfo.clearValue = clearValues[1];

      VkRenderingInfoKHR renderInfo{};
      renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
      renderInfo.renderArea.extent = exeParams.rc->swapChainExtent();
      renderInfo.renderArea.offset = { 0, 0 };
      renderInfo.layerCount = 1;
      renderInfo.colorAttachmentCount = static_cast<uint32_t>(colInfo.size());
      renderInfo.pColorAttachments = colInfo.data();
      renderInfo.pDepthAttachment = &depthAttachmentInfo;

      vkCmdBeginRendering(*exeParams.cmdBuffer, &renderInfo);

      // Viewport and scissor
      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = static_cast<float>(exeParams.rc->swapChainExtent().width);
      viewport.height = static_cast<float>(exeParams.rc->swapChainExtent().height);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      VkRect2D scissor{};
      scissor.offset = { 0, 0 };
      scissor.extent = exeParams.rc->swapChainExtent();

      // Bind pipeline
      vkCmdBindPipeline(*exeParams.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *exeParams.pipeline);

      // Descriptor set for translation buffer in set 1
      vkCmdBindDescriptorSets(
        *exeParams.cmdBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        *exeParams.pipelineLayout,
        1, 1, &(*exeParams.descriptorSets)[exeParams.rc->getCurrentMultiBufferIdx()],
        0, nullptr);

      // Set dynamic viewport and scissor
      vkCmdSetViewport(*exeParams.cmdBuffer, 0, 1, &viewport);
      vkCmdSetScissor(*exeParams.cmdBuffer, 0, 1, &scissor);

      // Ask render context to draw big giga buffer
      exeParams.rc->drawGigaBufferIndirectCount(
        exeParams.cmdBuffer, 
        exeParams.buffers[1],
        exeParams.buffers[4],
        static_cast<uint32_t>(exeParams.rc->getCurrentMeshes().size()));

      // Stop dynamic rendering
      vkCmdEndRendering(*exeParams.cmdBuffer);
    }
  );
}

}