#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char **argv) {
	void *handle1, *handle2, *handle3;
	int (*func_p)();

	handle1 = dlopenCheck("func1.so");
	handle2 = dlopenCheck("func2.so");
	handle3 = thisProcess();

	*(void **) (&func_p) = dlsymCheck(handle3, "func1");
	if (func_p)
		(*func_p)();

	*(void **) (&func_p) = dlsymCheck(handle3, "func2");
	if (func_p)
		(*func_p)();

	*(void **) (&func_p) = dlsymCheck(handle3, "func3");
	if (func_p)
		(*func_p)();

	*(void **) (&func_p) = dlsymCheck(handle2, "func2");
	if (func_p)
		(*func_p)();

	*(void **) (&func_p) = dlsymCheck(handle3, "funcM");
	if (func_p)
		(*func_p)();

	dlclose(handle3);
	dlclose(handle2);
	dlclose(handle1);
	exit(EXIT_SUCCESS);
}

int funcM () {
	printf ("funcM\n");
	return 1;
}

