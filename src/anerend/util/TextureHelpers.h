#pragma once

#include "../render/asset/Texture.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace util {

struct TextureHelpers
{

  static render::asset::Texture createTextureRGBA8(unsigned w, unsigned h, glm::u8vec4 val);

  static render::asset::Texture createTextureR8(unsigned w, unsigned h, std::uint8_t val);

  // 1 byte per channel
  static std::vector<std::uint8_t> convertRGBA8ToRGB8(std::vector<std::uint8_t> in);

  // 1 byte per channel, channels {1, 2} means GB to RG
  static std::vector<std::uint8_t> convertRGBA8ToRG8(std::vector<std::uint8_t> in, std::vector<unsigned> channels = { 1, 2 });

  // Output will also be 4 component
  static void convertRGBA8ToBC7(render::asset::Texture& tex);

  // Output will be 2 component
  static void convertRG8ToBC5(render::asset::Texture& tex);

};

}