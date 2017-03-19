//
// Copyright (c) Mario Garcia, MIT License.
//
#include "base.hpp"
#include <GLFW/glfw3.h>

#include <string>


namespace pbr {
namespace global {


VkInstance instance;


VkInstance GetInstance()
{
  return instance;
}


void KeyCallback(Window window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}
} // global


Base::Base()
{
  glfwInit();
}


void Base::SetupWindow(uint32_t width, uint32_t height)
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  m_window = glfwCreateWindow(width, height, "PBR Test Vulkan", nullptr, nullptr);
  glfwMakeContextCurrent(m_window);

  glfwSetKeyCallback(m_window, global::KeyCallback);
#if defined(_WIN32)
  HWND console = GetConsoleWindow();
  ShowWindow(console, SW_HIDE);
#endif
}


void Base::CloseWindow()
{
  if (m_window) {
    glfwSetWindowShouldClose(m_window, GL_TRUE);
  }
  glfwDestroyWindow(m_window);
}


void Base::Initialize()
{
}


void Base::Run()
{
  while (!glfwWindowShouldClose(m_window)) {
    double t = glfwGetTime();
    m_dt = t - m_lastTime;
    m_lastTime = t;
    glfwPollEvents();
    std::string str(std::to_string(m_dt) + " delta ms");
    glfwSetWindowTitle(m_window, str.c_str());

    
    glfwSwapBuffers(m_window); 
  }
  
  glfwTerminate();
}


void Base::Cleanup()
{
}
} // pbr