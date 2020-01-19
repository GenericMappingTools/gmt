/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

GMT_LOCAL char **breakMe(struct GMT_CTRL *GMT, char *in) {
	/* Breake a string "-aa -bb -cc dd" into tokens "-aa" "-bb" "-cc dd" */
	/* Based on GMT_Create_Options() */
	unsigned int pos = 0, n_args = 0, k;
	bool quoted;
	size_t n_alloc = GMT_SMALL_CHUNK;
	char p[GMT_BUFSIZ] = {""}, *txt_in = strdup (in);	/* Passed a single text string */
	char **args = NULL;
	args = gmt_M_memory (GMT, NULL, n_alloc, char *);
	/* txt_in can contain options that take multi-word text strings, e.g., -B+t"My title".  We avoid the problem of splitting
	 * these items by temporarily replacing spaces inside quoted strings with ASCII 31 US (Unit Separator), do the strtok on
	 * space, and then replace all ASCII 31 with space at the end (we do the same for tab using ASCII 29 GS (group separator) */
	for (k = 0, quoted = false; txt_in[k]; k++) {
		if (txt_in[k] == '\"') quoted = !quoted;	/* Initially false, becomes true at start of quote, then false when exit quote */
		else if (quoted && txt_in[k] == '\t') txt_in[k] = GMT_ASCII_GS;
		else if (quoted && txt_in[k] == ' ')  txt_in[k] = GMT_ASCII_US;
	}
	while ((gmt_strtok (txt_in, " ", &pos, p))) {	/* Break up string into separate words, and strip off double quotes */
		int i, o;
		for (k = 0; p[k]; k++)
			if (p[k] == GMT_ASCII_GS) p[k] = '\t'; else if (p[k] == GMT_ASCII_US) p[k] = ' ';	/* Replace spaces and tabs masked above */
		for (i = o = 0; p[i]; i++)
			if (p[i] != '\"') p[o++] = p[i];	/* Ignore double quotes */
		p[o] = '\0';
		if (p[0] == '-')
			args[n_args++] = strdup(p);
		else {		/* If string doesn't start with a '-' it means it's an argument to the option and must be packed together */
			args[n_args] = (char *)realloc(args[n_args], strlen(args[n_args])+strlen(p)+3);	/* Make room to the to be appended string */
			strcat(args[n_args], " ");
			strcat(args[n_args], p);
		}

		if (n_args == n_alloc) {
			n_alloc += GMT_SMALL_CHUNK;
			args = gmt_M_memory(GMT, args, n_alloc, char *);
		}
	}
	for (k = 0; txt_in[k]; k++)	/* Restore input string to prestine condition */
		if (txt_in[k] == GMT_ASCII_GS) txt_in[k] = '\t';
		else if (txt_in[k] == GMT_ASCII_US) txt_in[k] = ' ';	/* Replace spaces and tabs masked above */
	args[n_args] = NULL;	/* Close the list with a NULL */
	gmt_M_str_free (txt_in);
	return args;
}

GMT_LOCAL int grid_gdal_librarified (struct GMT_CTRL *GMT, char *gdal_filename, char *opts) {
	char	*info = NULL, **args;
	GDALDatasetH	hDataset;
	GDALInfoOptions *psOptions;

	/* Open gdal - */

	GDALAllRegister();

	hDataset = GDALOpen(gdal_filename, GA_ReadOnly);

	if (hDataset == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GDALOpen failed %s\n", CPLGetLastErrorMsg());
		return (-1);
	}

	args = breakMe(GMT, opts);
	psOptions = GDALInfoOptionsNew(args, NULL);
	info = GDALInfo(hDataset, psOptions);
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GDAL Info\n\n%s\n", info);

	GDALInfoOptionsFree(psOptions);
	GDALClose(hDataset);
	GDALDestroyDriverManager();
	return 0;
}
