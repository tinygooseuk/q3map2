/* marker */
#define CONVERT_MAP_UNITY_C

/* dependencies */
#include "q3map2.h"

typedef void (*OutputFunc)(const char* text);
OutputFunc g_OutputFunc = NULL;

// Disable for WASM
#if !defined( __EMSCRIPTEN__ ) && !defined( __wasi__ )

#include <setjmp.h>


#if defined( WIN32 )
#define DLL_EXPORT __declspec(dllexport) 
#else
#define DLL_EXPORT
#endif


jmp_buf g_ErrorHandler;

int main(int argc, char** argv);

DLL_EXPORT int convert_map_unity(const char* mapFile, const char* fsPath, OutputFunc outputFunc, const char** inArgs, int inArgsCount)
{
	int result = setjmp(g_ErrorHandler);
	if (result != 0)
	{
		// We errored somewhere deep - return error code
		return result;
	}

	// Read args into array
	char* args[1024] = {0};
	//args[0] = mapFile; //TODO(jwf): check this - Linux requires it!

	int argPtr = 0;

	args[argPtr++] = "-fs_basepath";
	args[argPtr++] = fsPath;

	for (int i = 0; i < inArgsCount; i++)
	{
		args[argPtr++] = inArgs[i];
	}

	args[argPtr++] = mapFile;

	// Create output handler
	g_OutputFunc = outputFunc;

    // Run!	
	ResetGlobals(); // Ewww...
	result = main(argPtr, args);

	return result;
}
#endif