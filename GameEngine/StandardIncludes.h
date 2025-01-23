#pragma once

#ifndef STANDARD_INCLUDES_H
#define STANDARD_INLCUDES_H

//Inlcude standard headrs
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cinttypes>
#include <stdio.h>
#include <thread>

//Poject headers
#include "Singleton.h"

//OS specific includes and defines
#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#define M_ASSERT(_cond, _msg) \
	if (!(_cond)) {OutputDebugStringA(_msg); std::abort();}
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

using namespace std;

#endif //STANDARD_INCLUDES_H
