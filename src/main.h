/**
	Header file includes
*/
#include <windows.h>
#include "cviscraft.h"

/**
	Advanced memory leek detection
*/
#ifdef _DEBUG
	#include <crtdbg.h>
	#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
	#pragma warning(disable : 4345)
#endif
