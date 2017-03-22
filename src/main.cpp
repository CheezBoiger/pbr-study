//
// Copyright (c) Mario Garcia, MIT License.
//
#include "renderer.hpp"
#include "platform.hpp"
#include "base.hpp"

#include <iostream>

int main(int c, char *argv[]) {
  std::cout << "Physically Based Rendering Study...\n";
  std::cout << "PBR_VERSION: " << PBR_CURRENT_VERSION << "\n";
  pbr::Base base;
  base.SetupWindow(1440, 900);
  base.Initialize();
  base.Run();
  return 0;
}