#pragma once

#include <sstream>
#include <fstream>
#include <filesystem>

#include <vulkan/vulkan.h>

#include "AllocatedImage.h"
#include "asset/Texture.h"

#include "../../common/util/nanojpeg.h"
#include "../../common/util/Utils.h"
#include <lodepng.h>

namespace render {
namespace imageutil {

static bool hasStencilComponent(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

static unsigned stride(render::asset::Texture::Format format)
{
  if (format == asset::Texture::Format::RGBA8_SRGB) {
    return 4;
  }
  else if (format == asset::Texture::Format::RGBA8_UNORM) {
    return 4;
  }
  else if (format == asset::Texture::Format::RGBA16F_SFLOAT) {
    return 8;
  }
  else if (format == asset::Texture::Format::RGB8_SRGB) {
    return 3;
  }
  else if (format == asset::Texture::Format::RG8_UNORM) {
    return 2;
  }
  else if (format == asset::Texture::Format::RGB8_UNORM) {
    return 3;
  }
  else if (format == asset::Texture::Format::RGBA_SRGB_BC7) {
    return 4;
  }
  else if (format == asset::Texture::Format::RGBA_UNORM_BC7) {
    return 4;
  }
  else if (format == asset::Texture::Format::RG_UNORM_BC5) {
    return 2;
  }
  else if (format == asset::Texture::Format::R8_UNORM) {
    return 1;
  }
  else if (format == asset::Texture::Format::R16_UNORM) {
    return 2;
  }

  return 4;
}

static VkFormat texFormatToVk(render::asset::Texture::Format format)
{
  if (format == asset::Texture::Format::RGBA8_SRGB) {
    return VK_FORMAT_R8G8B8A8_SRGB;
  }
  else if (format == asset::Texture::Format::RGBA8_UNORM) {
    return VK_FORMAT_R8G8B8A8_UNORM;
  }
  else if (format == asset::Texture::Format::RGBA16F_SFLOAT) {
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  }
  else if (format == asset::Texture::Format::RGB8_SRGB) {
    return VK_FORMAT_R8G8B8_SRGB;
  }
  else if (format == asset::Texture::Format::RG8_UNORM) {
    return VK_FORMAT_R8G8_UNORM;
  }
  else if (format == asset::Texture::Format::RGB8_UNORM) {
    return VK_FORMAT_R8G8B8_UNORM;
  }
  else if (format == asset::Texture::Format::RGBA_SRGB_BC7) {
    return VK_FORMAT_BC7_SRGB_BLOCK;
  }
  else if (format == asset::Texture::Format::RGBA_UNORM_BC7) {
    return VK_FORMAT_BC7_UNORM_BLOCK;
  }
  else if (format == asset::Texture::Format::RG_UNORM_BC5) {
    return VK_FORMAT_BC5_UNORM_BLOCK;
  }
  else if (format == asset::Texture::Format::R8_UNORM) {
    return VK_FORMAT_R8_UNORM;
  }
  else if (format == asset::Texture::Format::R16_UNORM) {
    return VK_FORMAT_R16_UNORM;
  }

  return VK_FORMAT_R8G8B8A8_UNORM;
}

static unsigned numDimensions(render::asset::Texture::Format format)
{
  if (format == asset::Texture::Format::RGBA8_SRGB) {
    return 4;
  }
  else if (format == asset::Texture::Format::RGBA8_UNORM) {
    return 4;
  }
  else if (format == asset::Texture::Format::RGBA16F_SFLOAT) {
    return 4;
  }
  else if (format == asset::Texture::Format::RGB8_SRGB) {
    return 3;
  }
  else if (format == asset::Texture::Format::RG8_UNORM) {
    return 2;
  }
  else if (format == asset::Texture::Format::RGB8_UNORM) {
    return 3;
  }
  else if (format == asset::Texture::Format::RGBA_SRGB_BC7) {
    return 4;
  }
  else if (format == asset::Texture::Format::RGBA_UNORM_BC7) {
    return 4;
  }
  else if (format == asset::Texture::Format::RG_UNORM_BC5) {
    return 2;
  }
  else if (format == asset::Texture::Format::R8_UNORM) {
    return 1;
  }
  else if (format == asset::Texture::Format::R16_UNORM) {
    return 1;
  }

  return 3;
}

static std::size_t texSize(int w, int h, render::asset::Texture::Format format)
{
  if (format == asset::Texture::Format::RGBA8_SRGB) {
    return w * h * 1 * 4;
  }
  else if (format == asset::Texture::Format::RGBA16F_SFLOAT) {
    return w * h * 2 * 4;
  }
  else if (format == asset::Texture::Format::RGB8_SRGB) {
    return w * h * 1 * 3;
  }
  else if (format == asset::Texture::Format::RG8_UNORM) {
    return w * h * 1 * 2;
  }
  else if (format == asset::Texture::Format::RGB8_UNORM) {
    return w * h * 1 * 3;
  }
  else if (format == asset::Texture::Format::RGBA_SRGB_BC7) {
    return w * h * 1;
  }
  else if (format == asset::Texture::Format::RGBA_UNORM_BC7) {
    return w * h * 1;
  }
  else if (format == asset::Texture::Format::RG_UNORM_BC5) {
    return w * h * 1;
  }

  return w * h * 1 * 4;
}

static void createImage(
  uint32_t width,
  uint32_t height,
  VkFormat format,
  VkImageTiling tiling,
  VmaAllocator allocator,
  VkImageUsageFlags usage,
  AllocatedImage& image,
  uint32_t mipLevels = 1,
  uint32_t arrayLayers = 1,
  VkImageCreateFlags flags = 0,
  bool hostAccess = false,
  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED)
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = arrayLayers;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = initialLayout;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.flags = flags;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  if (hostAccess) {
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
  }

  if (vmaCreateImage(allocator, &imageInfo, &vmaAllocInfo, &image._image, &image._allocation, nullptr) != VK_SUCCESS) {
    printf("Failed to create image!\n");
  }
}

namespace
{

// Convert RGB to RGBA
std::vector<uint8_t> uglyFixData(const std::vector<std::uint8_t>& rgb)
{
  std::vector<uint8_t> output;
  //output.resize(rgb.size() / 3 + rgb.size());

  for (std::size_t i = 0; i < rgb.size();) {
    output.emplace_back(rgb[i + 0]);
    output.emplace_back(rgb[i + 1]);
    output.emplace_back(rgb[i + 2]);
    output.emplace_back(255);

    i = i + 3;
  }

  return output;
}

}

struct TextureData
{
  explicit operator bool() const { return !data.empty(); }

  std::vector<uint8_t> data;
  int width;
  int height;
  int bitdepth = 8;
  bool isColor;
};

static void init()
{
}

static TextureData loadTex(const std::string& tex, bool requestGray = false)
{
  TextureData output{};

  std::filesystem::path p(tex);
  auto extension = p.extension().string();

  if (extension == ".jpg") {
    njInit();

    std::ifstream ifs(p, std::ios::binary);
    if (!ifs.is_open()) {
      printf("Could not open jpeg: %s\n", tex.c_str());
      njDone();
      return output;
    }

    std::ostringstream stream;
    stream << ifs.rdbuf();

    if (njDecode((void*)stream.str().data(), (int)stream.str().size())) {
      printf("Could not decode jpeg: %s\n", tex.c_str());
      njDone();
      return output;
    }

    bool isColor = njIsColor();
    int width = njGetWidth();
    int height = njGetHeight();

    output.data.resize(njGetImageSize());
    std::memcpy(output.data.data(), njGetImage(), njGetImageSize());

    if (isColor) {
      auto fixed = uglyFixData(output.data);
      output.data = std::move(fixed);
    }

    output.isColor = isColor;
    output.width = width;
    output.height = height;

    njDone();
  }
  else if (extension == ".png") {
    unsigned width, height;

    auto readFile = util::readFile(tex);
    LodePNGState state{};
    lodepng_inspect(&width, &height, &state, (unsigned char*)readFile.data(), readFile.size());

    printf("Bitdepth of png: %u\n", state.info_png.color.bitdepth);

    LodePNGColorType colorType = requestGray ? LodePNGColorType::LCT_GREY : LodePNGColorType::LCT_RGBA;
    unsigned error = lodepng::decode(output.data, width, height, tex.c_str(), colorType, state.info_png.color.bitdepth);
    if (error != 0) {
      printf("Failed to decode png: %s\n", tex.c_str());
      printf("Error is: %s\n", lodepng_error_text(error));
      return output;
    }

    // If 16 bit, lodepng for some reason likes its endianness the other way around
    if (state.info_png.color.bitdepth == 16) {
      std::vector<std::uint8_t> temp;
      temp.resize(output.data.size());

      for (unsigned x = 0; x < width; ++x) {
        for (unsigned y = 0; y < height; ++y) {
          temp[2 * width * y + 2 * x + 0] = output.data[2 * width * y + 2 * x + 1];
          temp[2 * width * y + 2 * x + 1] = output.data[2 * width * y + 2 * x + 0];
        }
      }

      output.data = std::move(temp);
    }

    output.width = width;
    output.height = height;
    output.bitdepth = state.info_png.color.bitdepth;
    output.isColor = !requestGray;
  }
  else {
    printf("Unsupported extension in loadTex: %s\n", extension.c_str());
    return output;
  }

  printf("Decoded texture %s (isColor: %d, width: %d, height: %d)\n", tex.c_str(), output.isColor, output.width, output.height);

  return output;
}

static VkImageView createImageView(
  VkDevice device,
  VkImage image,
  VkFormat format,
  VkImageAspectFlags aspectFlags,
  uint32_t baseMipLevel = 0,
  uint32_t mipLevels = 1,
  uint32_t baseLayer = 0,
  uint32_t layerCount = 1,
  VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D)
{
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = viewType;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = baseLayer;
  viewInfo.subresourceRange.layerCount = layerCount;

  VkImageView imageView;
  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    printf("failed to create texture image view!\n");
  }

  return imageView;
}

static void transitionImageLayout(
  VkCommandBuffer commandBuffer,
  VkImage image,
  VkFormat format,
  VkImageLayout oldLayout,
  VkImageLayout newLayout,
  uint32_t baseMipLevel = 0,
  uint32_t mipLevels = 1,
  uint32_t baseLayer = 0,
  uint32_t layerCount = 1)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = baseMipLevel;
  barrier.subresourceRange.levelCount = mipLevels;
  barrier.subresourceRange.baseArrayLayer = baseLayer;
  barrier.subresourceRange.layerCount = layerCount;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
  else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    destinationStage = VK_PIPELINE_STAGE_HOST_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
  else {
    printf("unsupported layout transition!\n");
  }

  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
      newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
      format == VK_FORMAT_D32_SFLOAT) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (hasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }
  else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );
}

}}