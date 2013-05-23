#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_plugin.h"

/* Handle for this process */
DLHANDLE thisProcess () {
#ifdef _WIN32
	HMODULE this_process;
	GetModuleHandleEx (0, 0, &this_process);
	return GetModuleHandle(0);
#else
	return dlopen(NULL,0);
#endif
}

/* Try to dynamically load library and check load status. */
DLHANDLE dlopenCheck (const char *pluginPath) {
	DLHANDLE handle = dlopen (pluginPath, RTLD_LAZY);

	if (!handle) {
#ifdef _WIN32
		fprintf(stderr, "Error loading plugin %s\n", pluginPath);
#else
		fprintf(stderr, "Error loading plugin %s: %s\n", pluginPath, dlerror());
#endif
		return NULL;
	}
	return handle;
}

/* Try to get function pointer. */
void* dlsymCheck (DLHANDLE handle, const char *symbol) {
	void* ptr;
	dlerror(); /* Clear any existing error */
	ptr = dlsym(handle, symbol);
#ifdef _WIN32
	if (ptr == NULL) {
		fprintf(stderr, "Error loading symbol %s\n", symbol);
		return NULL;
	}
#else
	{
		char* error;
		if ((error = dlerror()) != NULL)  {
			fprintf(stderr, "Error loading symbol %s: %s\n", symbol, error);
			return NULL;
		}
	}
#endif
	return ptr;
}

/*
struct Gmt_plugin {
	const char *path;
	DLHANDLE *handle;
	struct Gmt_plugin *next;
};
*/
struct Gmt_plugin *loadPlugin (struct Gmt_plugin *plugin, const char *path) {
	struct Gmt_plugin *new;
	DLHANDLE handle;

	if (plugin == NULL)
		return NULL;

	handle = dlopenCheck(path); /* get handle */
	if (handle == NULL)
		return NULL;

	while (plugin->next != NULL) /* advance to last plugin */
		plugin = plugin->next;

	/* add new handle */
	new = malloc (sizeof(struct Gmt_plugin));
	new->path = strdup (path);
	new->handle = handle;
	new->next = NULL;
	plugin->next = new;

	return new;
}

void unloadPlugins (struct Gmt_plugin *plugin) {
	if (plugin->next != NULL)
		unloadPlugins (plugin->next);
	dlclose (plugin->handle);
	free (plugin);
}

void *ptrSym (struct Gmt_plugin *plugin, const char* symbol) {
	void *ptr;

	while (plugin != NULL) {
		ptr = dlsymCheck (plugin->handle, symbol);
		if (ptr != NULL) {
			fprintf (stderr, "found symbol %s in plugin %s\n", symbol, plugin->path);
			return ptr; /* found symbol */
		}
		plugin = plugin->next;
	}

	return NULL;
}


