#pragma once

#include <stdint.h>

// Define static assertions
#if defined(__clang__) || defined(__gcc__)
	#define STATIC_ASSERT _Static_assert
#else
	#define STATIC_ASSERT static_assert
#endif

#ifdef L_EXPORT
	#ifdef L_ISWIN
		#define LAPI __declspec(dllexport)
	#else
		#define LAPI __attribute__((visibility("default")))
	#endif
#else
	#ifdef L_ISWIN
		#define LAPI __declspec(dllimport)
	#else
		#define LAPI 
	#endif
#endif
