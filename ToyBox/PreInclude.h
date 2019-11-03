#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <random>

#include "SceneMgr.h"
#include <pthread.h>
#include "Renderer.h"
#include "UIController.h"
#include "ShaderMgr.h"

//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
GLFWwindow* InitWnd();

// static param init
bool MainUI::show_control_window = true;
bool MainUI::show_demo_window = false;
bool MainUI::show_VFB_window = false;
bool MainUI::show_preference_window = false;
bool MainUI::show_about_window = false;

GLuint MainUI::iconID = -1;
GLuint MainUI::logoID = -1;
int MainUI::iconWidth = 0;
int MainUI::iconHeight = 0;
int MainUI::logoWidth = 0;
int MainUI::logoHeight = 0;