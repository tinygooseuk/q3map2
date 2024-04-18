/* marker */
#define CONVERT_MAP_UNITY_C


/* dependencies */
#include "q3map2.h"

#if defined( WIN32 )
#define DLL_EXPORT __declspec(dllexport) 
#else
#define DLL_EXPORT
#endif

FILE* g_OutputFile = NULL;

int main(int argc, char** argv);

DLL_EXPORT int convert_map_unity(const char* mapFile, const char* fsPath, const char* outputFile, const char** inArgs, int inArgsCount)
{
	// Read args into array
	char* args[1024] = {0};
	args[0] = mapFile;

	int argPtr = 1;

	args[argPtr++] = "-fs_basepath";
	args[argPtr++] = fsPath;

	for (int i = 0; i < inArgsCount; i++)
	{
		args[argPtr++] = inArgs[i];
	}

	args[argPtr++] = mapFile;

	// Create output file
	if (g_OutputFile)
	{
		fclose(g_OutputFile);
		g_OutputFile = NULL;
	}
	g_OutputFile = fopen(outputFile, "wb");

    // Run!
	int result = main(argPtr, args);

    // Close output file
	if (g_OutputFile)
	{
		fclose(g_OutputFile);
		g_OutputFile = NULL;
	}
	return result;
}