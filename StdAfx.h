#pragma once

#ifndef _DEBUG
#define _SECURE_SCL	0
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <malloc.h>
#include <strsafe.h>
#include <crtdbg.h>
#include <commctrl.h>
#pragma warning( push )
#pragma warning( disable : 4995 ) // 'function': name was marked as #pragma deprecated
#include <shlwapi.h>
#pragma warning( pop )

#include <process.h>

#pragma warning( push )
#pragma warning( disable : 4995 ) // 'function': name was marked as #pragma deprecated
#pragma warning( disable : 4244 )  // 'argument' : conversion from '__w64 unsigned int' to 'rsize_t', possible loss of data
#include <strsafe.h>
#include <vector>
#include <string>
#pragma warning( pop )

using namespace std;


#ifdef _DEBUG
#define VERIFY(f)          _ASSERTE(f)
#else
#define VERIFY(f)          ((void)(f))
#endif

#ifdef _DEBUG
   #define _DEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
   #define _RELEASE_NEW
#else
   #define _DEBUG_NEW
   #define _RELEASE_NEW new(nothrow)
#endif // _DEBUG
