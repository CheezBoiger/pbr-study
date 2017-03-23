//
// Copyright (c) Mario Garcia, MIT License.
//
#include "renderer.hpp"
#include "platform.hpp"
#include "base.hpp"
#include "vector.hpp"

#include <iostream>

int main(int c, char *argv[]) {
  std::printf("%d bytes\n", static_cast<int32_t>(sizeof(pbr::Vec2)));
  std::cout << "Physically Based Rendering Study...\n";
  std::cout << "PBR_VERSION: " << PBR_CURRENT_VERSION << "\n";
  pbr::Base base;
  base.SetupWindow(1440, 900);
  base.Initialize();
  base.Run();
  return 0;
}