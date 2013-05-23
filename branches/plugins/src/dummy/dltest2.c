#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_plugin.h"

int main(int argc, char **argv) {
	struct Gmt_plugin plugins;
	int (*func_p)();

	plugins.next = NULL;
	plugins.handle = thisProcess();

	loadPlugin (&plugins, "func1.so");
	loadPlugin (&plugins, "func2.so");

	*(void **) (&func_p) = ptrSym(&plugins, "func1");
	if (func_p)
		(*func_p)();

	*(void **) (&func_p) = ptrSym(&plugins, "func2");
	if (func_p)
		(*func_p)();

	/* this symbol does not exist */
	*(void **) (&func_p) = ptrSym(&plugins, "func3");
	if (func_p)
		(*func_p)();

	*(void **) (&func_p) = ptrSym(&plugins, "func2");
	if (func_p)
		(*func_p)();

	*(void **) (&func_p) = ptrSym(&plugins, "funcM");
	if (func_p)
		(*func_p)();

	unloadPlugins (plugins.next);
	dlclose(plugins.handle);
	exit(EXIT_SUCCESS);
}

int funcM () {
	printf ("funcM\n");
	return 1;
}

