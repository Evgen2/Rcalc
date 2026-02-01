// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifdef _WIN32

#pragma once

#define  _CRT_SECURE_NO_WARNINGS 1

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <io.h>
#include <conio.h>
#include <locale.h>
#include <math.h>


// TODO: reference additional headers your program requires here
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#endif // _WIN32