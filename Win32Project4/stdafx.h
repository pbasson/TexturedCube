// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <vector>
using namespace std;

#include <d3d11_1.h>
#include <d3d11.h>


#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include "SimpleMath.h"

using namespace DirectX;
using namespace SimpleMath; 

#define _USE_MATH_DEFINES
#include <math.h>

