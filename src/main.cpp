//
// Copyright (c) Mario Garcia, MIT License.
//
#include "base.hpp"

#include <iostream>

int main(int c, char *argv[]) {
  std::cout << "PBR_VERSION: " << PBR_CURRENT_VERSION << "\n";
  std::cout << R"(
    StupidEngine (TM) PBR Render Sample
    Copyright (c) Mario Garcia, MIT License.

    Keys:
    W = Move camera forward
    A = Move camera left 
    S = Move camera back
    D = Move camera right
    Q = Move camera down
    E = Move camera up
  
    Hold M/N = Increase/Decrease metallic value
    Hold R/T = Increase/Decrease roughness value
  
    L/O = Turn on/off lights.
    
    ESC = quit.

  )";
  std::cout << "\nPress Enter to start up the renderer.\n";
  std::cin.ignore();
  std::cout << "Starting up...\n";
  pbr::Base base;
  base.SetupWindow(1440, 900);
  base.Initialize();
  std::cout << "Complete!\n";
  base.Run();
  std::cout << "Exiting.\n";
  return 0;
}