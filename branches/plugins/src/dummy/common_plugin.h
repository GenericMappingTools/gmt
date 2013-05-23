#ifdef _WIN32
#include <Windows.h>
#define dlopen(a, b) LoadLibrary(a)
#define dlsym(a, b) GetProcAddress(a, b)
#define dlerror() ((void)0)
#define dlclose(a) FreeLibrary(a)
#define DLHANDLE HMODULE
#else
#include <dlfcn.h>
#define DLHANDLE void*
#endif

/* linked list of plugins */
struct Gmt_plugin;
struct Gmt_plugin {
	const char *path;        /* for dlopen */
	DLHANDLE *handle;          /* for dlsym */
	struct Gmt_plugin *next; /* next plugin */
};

DLHANDLE thisProcess ();
DLHANDLE dlopenCheck (const char *pluginPath);
void* dlsymCheck (DLHANDLE handle, const char *symbol);
struct Gmt_plugin *loadPlugin (struct Gmt_plugin *plugin, const char *path);
void unloadPlugins (struct Gmt_plugin *plugin);
void *ptrSym (struct Gmt_plugin *plugin, const char* symbol);

