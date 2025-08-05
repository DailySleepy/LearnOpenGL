#pragma once
#include "GL\glew.h"
#include <glm.hpp>
#include <iostream>
using namespace std;
using namespace glm;

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) clearError(); x; ASSERT(!checkError());
void clearError();
bool checkError();