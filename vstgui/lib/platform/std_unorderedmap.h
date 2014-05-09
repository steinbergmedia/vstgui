#pragma once

#include "../vstguibase.h"

#if MAC
	#ifdef _LIBCPP_VERSION
		#include <unordered_map>
	#else
		#include <tr1/unordered_map>
		namespace std { using tr1::unordered_map; }
	#endif
#elif WINDOWS
	#include <unordered_map>
#endif
