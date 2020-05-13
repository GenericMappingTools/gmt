/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-Jan-2018
 * Version:	6 API
 *
 * Brief synopsis: gmt batch automates batch processing
 *
 * batch automates the main processing loop and much of the machinery needed
 * to script a batch sequence.  It allows for an optional preflight
 * and post-run scripts to be specified via separate modern scripts and
 * a single processing script that uses special variables to use slightly
 * different parameters for each job.  The user only needs to compose these
 * simple one-job scripts and then batch takes care of the automation.
 * Jobs are run in parallel without need for OpenMP etc.
 */

#include "gmt_dev.h"
#ifdef WIN32
#include <windows.h>
#endif

#define THIS_MODULE_CLASSIC_NAME	"batch"
#define THIS_MODULE_MODERN_NAME	"batch"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Automate batch job processing"
#define THIS_MODULE_KEYS	"<D("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-Vf"

#define BATCH_PREFLIGHT		0
#define BATCH_POSTFLIGHT	1

#define BATCH_WAIT_TO_CHECK	10000	/* In microseconds, so 0.01 seconds */
#define BATCH_PAUSE_A_SEC	1000000	/* In microseconds, so 1 seconds */

enum enum_script {BASH_MODE = 0,	/* Write Bash script */
	CSH_MODE,			/* Write C-shell script */
	DOS_MODE};			/* Write DOS script */

/* Control structure for batch */

struct BATCH_CTRL {
	struct BATCH_In {	/* mainscript (Bourne, Bourne Again, csh, or DOS (bat) script) */
		bool active;
		enum enum_script mode;
		char *file;	/* Name of main script */
		FILE *fp;	/* Open file pointer to main script */
	} In;
	struct BATCH_I {	/* -I<includefile> */
		bool active;
		char *file;	/* Name of include script */
		FILE *fp;	/* Open file pointer to include script */
	} I;
	struct BATCH_M {	/* -M[<job>] */
		bool active;
		bool exit;
		unsigned int job;	/* Job selected as master job */
	} M;
	struct BATCH_N {	/* -N<batchprefix> */
		bool active;
		char *prefix;	/* Job prefix and also name of working directory (but see -W) */
	} N;
	struct BATCH_Q {	/* -Q[s] */
		bool active;
		bool scripts;
	} Q;
	struct BATCH_S {	/* -Sb|f<script> */
		bool active;
		char *file;	/* Name of script file */
		FILE *fp;	/* Open file pointer to script */
	} S[2];
	struct BATCH_T {	/* -T<n_jobs>|<min>/<max/<inc>[+n]|<timefile>[+p<precision>][+s<job>][+w] */
		bool active;
		bool split;		/* true means we must split any trailing text in to words */
		unsigned int n_jobs;	/* Total number of jobs */
		unsigned int start_job;	/* First job [0] */
		unsigned int precision;	/* Decimals used in making unique job tags */
		char *file;		/* timefile name */
	} T;
	struct BATCH_W {	/* -W<workingdirectory> */
		bool active;
		char *dir;	/* Alternative working directory than implied by -N */
	} W;
	struct BATCH_Z {	/* -Z[s] */
		bool active;	/* Delete temporary files when completed */
		bool delete;	/* Also delete all files including the mainscript and anything passed via  -I, -S */
	} Z;
	struct BATCH_x {	/* -x[[-]<ncores>] */
		bool active;
		int n_threads;
	} x;
};

struct BATCH_STATUS {
	/* Used to monitor the start, running, and completion of job jobs running in parallel */
	bool started;	/* true if job job has started */
	bool completed;	/* true if the completion file has been successfully produced */
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct BATCH_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct BATCH_CTRL);
	C->x.n_threads = GMT->parent->n_cores;	/* Use all cores available unless -x is set */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct BATCH_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->N.prefix);
	gmt_M_str_free (C->S[BATCH_PREFLIGHT].file);
	gmt_M_str_free (C->S[BATCH_POSTFLIGHT].file);
	gmt_M_str_free (C->T.file);
	gmt_M_str_free (C->W.dir);
	gmt_M_free (GMT, C);
}

/*! -x[[-]<ncores>] parsing needed but here not related to OpenMP etc - it is just a local option */
GMT_LOCAL int batch_parse_x_option (struct GMT_CTRL *GMT, struct BATCH_CTRL *Ctrl, char *arg) {
	if (!arg) return (GMT_PARSE_ERROR);	/* -x requires a non-NULL argument */
	if (arg[0])
		Ctrl->x.n_threads = atoi (arg);

	if (Ctrl->x.n_threads == 0)	/* Not having any of that.  At least one */
		Ctrl->x.n_threads = 1;
	else if (Ctrl->x.n_threads < 0)	/* Meant to reduce the number of threads */
		Ctrl->x.n_threads = MAX(GMT->parent->n_cores + Ctrl->x.n_threads, 1);		/* Max-n but at least one */
	return (GMT_NOERROR);
}

GMT_LOCAL int batch_gmt_sleep (unsigned int microsec) {
	/* Waiting before checking if the completion file has been generated */
#ifdef WIN32
	Sleep ((uint32_t)microsec/1000);	/* msec are microseconds but Sleep wants millisecs */
	return 0;
#else
	return (usleep ((useconds_t)microsec));
#endif
}

GMT_LOCAL void batch_set_value (struct GMT_CTRL *GMT, FILE *fp, int mode, int col, char *name, double value) {
	/* Assigns a single named data floating point variable given the script mode
	 * Here, col indicates which input column in case special formatting is implied via -f */
	char string[GMT_LEN64] = {""};
	gmt_ascii_format_one (GMT, string, value, gmt_M_type (GMT, GMT_IN, col));
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%s", name, string);       break;
		case CSH_MODE:  fprintf (fp, "set %s = %s", name, string); break;
		case DOS_MODE:  fprintf (fp, "set %s=%s", name, string);   break;
	}
	fprintf (fp, "\n");
}

GMT_LOCAL void batch_set_dvalue (FILE *fp, int mode, char *name, double value, char unit) {
	/* Assigns a single named Cartesian floating point variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%.12g", name, value);       break;
		case CSH_MODE:  fprintf (fp, "set %s = %.12g", name, value); break;
		case DOS_MODE:  fprintf (fp, "set %s=%.12g", name, value);   break;
	}
	if (unit) fprintf (fp, "%c", unit);	/* Append the unit [c|i|p] unless 0 */
	fprintf (fp, "\n");
}

GMT_LOCAL void batch_set_ivalue (FILE *fp, int mode, bool env, char *name, int value) {
	/* Assigns a single named integer variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%d\n", name, value);       break;
		case CSH_MODE:  if (env)
					fprintf (fp, "%s %d\n", name, value);
				else
					fprintf (fp, "set %s = %d\n", name, value);
				break;
		case DOS_MODE:  fprintf (fp, "set %s=%d\n", name, value);   break;
	}
}

GMT_LOCAL void batch_set_tvalue (FILE *fp, int mode, bool env, char *name, char *value) {
	/* Assigns a single named text variable given the script mode */
	if (strchr (value, ' ') || strchr (value, '\t') || strchr (value, '|')) {	/* String has spaces, tabs, or bar */
		switch (mode) {
			case BASH_MODE: fprintf (fp, "%s=\"%s\"\n", name, value);       break;
			case CSH_MODE:  if (env)
						fprintf (fp, "%s \"%s\"\n", name, value);
					else
						fprintf (fp, "set %s = \"%s\"\n", name, value);
					break;
			case DOS_MODE:  fprintf (fp, "set %s=\"%s\"\n", name, value);   break;
		}
	}
	else {	/* Single word */
		switch (mode) {
			case BASH_MODE: fprintf (fp, "%s=%s\n", name, value);       break;
			case CSH_MODE:  if (env)
						fprintf (fp, "%s %s\n", name, value);
					else
						fprintf (fp, "set %s = %s\n", name, value);
					break;
			case DOS_MODE:  fprintf (fp, "set %s=%s\n", name, value);   break;
		}
	}
}

GMT_LOCAL void batch_set_script (FILE *fp, int mode) {
	/* Writes the script's incantation line (or a comment for DOS, turning off default echo) */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "#!/usr/bin/env bash\n"); break;
		case CSH_MODE:  fprintf (fp, "#!/usr/bin/env csh\n"); break;
		case DOS_MODE:  fprintf (fp, "@echo off\nREM Start of script\n"); break;
	}
}

GMT_LOCAL void batch_set_comment (FILE *fp, int mode, char *comment) {
	/* Write a comment line given the script mode */
	switch (mode) {
		case BASH_MODE: case CSH_MODE:  fprintf (fp, "# %s\n", comment); break;
		case DOS_MODE:  fprintf (fp, "REM %s\n", comment); break;
	}
}

GMT_LOCAL char *batch_place_var (int mode, char *name) {
	/* Prints a single variable to stdout where needed in the script via the string static variable.
	 * PS!  Only one call per printf statement since static string cannot hold more than one item at the time */
	static char string[GMT_LEN128] = {""};	/* So max length of variable name is 127 */
	if (mode == DOS_MODE)
		sprintf (string, "%%%s%%", name);
	else
		sprintf (string, "${%s}", name);
	return (string);
}

GMT_LOCAL int batch_dry_run_only (const char *cmd) {
	/* Dummy function to not actually run the loop script when -Q is used */
	gmt_M_unused (cmd);
	return 0;
}

GMT_LOCAL unsigned int batch_check_language (struct GMT_CTRL *GMT, unsigned int mode, char *file, unsigned int k) {
	unsigned int n_errors = 0;
	size_t L;
	/* Examines file extension and compares to known mode from mainscript */

	switch (mode) {
		case BASH_MODE:
			if (!(strstr (file, ".bash") || strstr (file, ".sh"))) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Main script is bash/sh but %s is not!\n", file);
				n_errors++;
			}
			break;
		case CSH_MODE:
			if (!strstr (file, ".csh")) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Main script is csh but %s is not!\n", file);
				n_errors++;
			}
			break;
		case DOS_MODE:
			if (!strstr (file, ".bat")) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Main script is bat but %s is not!\n", file);
				n_errors++;
			}
			break;
	}
	return (n_errors);
}

GMT_LOCAL bool batch_script_is_classic (struct GMT_CTRL *GMT, FILE *fp) {
	/* Read script to determine if it is in GMT classic mode or not, then rewind */
	bool modern = false;
	char line[PATH_MAX] = {""};
	while (!modern && gmt_fgets (GMT, line, PATH_MAX, fp)) {
		if (strstr (line, "gmt ") == NULL) continue;	/* Does not start with gmt */
		if (strstr (line, " begin"))		/* A modern mode script */
			modern = true;
		else if (strstr (line, " figure"))	/* A modern mode script */
			modern = true;
		else if (strstr (line, " subplot"))	/* A modern mode script */
			modern = true;
		else if (strstr (line, " inset"))	/* A modern mode script */
			modern = true;
		else if (strstr (line, " end"))		/* A modern mode script */
			modern = true;
	}
	rewind (fp);	/* Go back to beginning of file */
	return (!modern);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <mainscript> -N<prefix> -T<njobs>|<min>/<max>/<inc>[+n]|<timefile>[+p<width>][+s<first>][+w]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<includefile>] [-M[<job>,]] [-Q[s]] [-Sb<post-run>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Sf<preflight>] [%s] [-W[<workdir>]] [-Z[s]] [%s] [-x[[-]<n>]] [%s]\n\n", GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<mainscript> is the main GMT modern script that completes a single job.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set the <prefix> used for batch files and directory names.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set number of jobs, create parameters from <min>/<max>/<inc>[+n] or give file with job-specific information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <min>/<max>/<inc> is used then +n is used to indicate that <inc> is in fact number of jobs instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <timefile> does not exist it must be created by the preflight script given via -Sf.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p<width> to set number of digits used in creating the job tags [automatic].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s<first> to change the value of the first job [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +w to <timefile> to have trailing text be split into individual word variables.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Include a script file to be inserted into the batch_init.sh script [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Used to add constant variables needed by all batch scripts.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Create a master job as well and run just this one for testing.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Debugging: Leave all intermediate files and directories behind for inspection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to only create the work scripts but none will be executed (except for post-run script).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Given names for the optional post-run and preflight GMT scripts [none]:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sb Append name of preflight GMT modern script that may download or compute\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       files needed by <mainscript>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sf Append name of post-run GMT modern mode script which will\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       take actions once all batch jobs have completed.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Give <workdir> where temporary files will be built [<workdir> = <prefix> set by -N].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <workdir> is not given we create one in the system temp directory named <prefix> (from -N).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Erase directory <prefix> after converting to batch [leave directory alone].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to also delete all input scripts (mainscript and any files via -I, -S)\n");
	GMT_Option (API, "f");
	/* Number of threads (re-purposed from -x in GMT_Option since this local option is always available and we are not using OpenMP) */
	GMT_Message (API, GMT_TIME_NONE, "\t-x Limit the number of cores used in job generation [Default uses all cores = %d].\n", API->n_cores);
	GMT_Message (API, GMT_TIME_NONE, "\t   -x<n>  Select <n> cores (up to all available).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -x-<n> Select (all - <n>) cores (or at least 1).\n");
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int batch_get_n_jobs (struct GMT_CTRL *GMT, char *txt, double jobrate, char *def) {
	/* Convert user argument in jobs or seconds to jobs */
	char *p = (!txt || !txt[0]) ? def : txt;	/* Get jobs or times in seconds */
	double fval = atof (p);	/* Get jobs or times in seconds */
	gmt_M_unused (GMT);
	if (strchr (p, 's')) fval *= jobrate;	/* Convert from seconds to nearest number of jobs */
	return (urint (fval));
}

static int parse (struct GMT_CTRL *GMT, struct BATCH_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to batch and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 */

	unsigned int n_errors = 0, n_files = 0, k, pos, mag, T, jobs;
	int n;
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""}, arg[GMT_LEN64] = {""}, p[GMT_LEN256] = {""};
	char *c = NULL, *s = NULL, string[GMT_LEN128] = {""};
	struct BATCH_ITEM *I = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_TEXT)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			case 'I':	/* Include file with settings used by all scripts */
				Ctrl->I.active = true;
				Ctrl->I.file = strdup (opt->arg);
				break;

			case 'M':	/* Create a single job as well as batch (unless -Q is active) */
				Ctrl->M.active = true;
				if (opt->arg[0])	/* Gave a job number */
					Ctrl->M.job = atoi (opt->arg);
				break;

			case 'N':	/* Movie prefix and directory name */
				Ctrl->N.active = true;
				Ctrl->N.prefix = strdup (opt->arg);
				break;

			case 'Q':	/* Debug - leave temp files and directories behind; Use -Qs to only write scripts */
				Ctrl->Q.active = true;
				if (opt->arg[0] == 's') Ctrl->Q.scripts = true;
				break;

			case 'S':	/* post-run and preflight scripts */
				if (opt->arg[0] == 'b')
					k = BATCH_PREFLIGHT;	/* post-run */
				else if (opt->arg[0] == 'f')
					k = BATCH_POSTFLIGHT;	/* preflight */
				else {	/* Bad option */
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -S: Select -Sb or -Sf\n");
					break;
				}
				/* Got a valid f or b */
				Ctrl->S[k].active = true;
				Ctrl->S[k].file = strdup (&opt->arg[1]);
				if ((Ctrl->S[k].fp = fopen (Ctrl->S[k].file, "r")) == NULL) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -S%c: Unable to open file %s\n", opt->arg[0], Ctrl->S[k].file);
					n_errors++;
				}
				break;

			case 'T':	/* Number of jobs or the name of file with job information (note: file may not exist yet) */
				Ctrl->T.active = true;
				if ((c = gmt_first_modifier (GMT, opt->arg, "psw"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'T', c, "psw", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'p':	/* Set a fixed precision in job naming ###### */
								Ctrl->T.precision = atoi (&p[1]);
								break;
							case 's':	/* Specify start job other than 0 */
								Ctrl->T.start_job = atoi (&p[1]);
								break;
							case 'w':	/* Split trailing text into words. */
								Ctrl->T.split = true;
								break;
							default:
								break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';
				}
				Ctrl->T.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore modifiers */
				break;

			case 'W':	/* Work dir where data files may be found. If not given we make one up later */
				Ctrl->W.active = true;
				if (opt->arg[0]) Ctrl->W.dir = strdup (opt->arg);
				break;

			case 'Z':	/* Erase jobs after batch has been made */
				Ctrl->Z.active = true;
				if (opt->arg[0] == 's') Ctrl->Z.delete = true;	/* Also delete input scripts */
				break;

			case 'x':
				n_errors += batch_parse_x_option (GMT, Ctrl, opt->arg);
				Ctrl->x.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1 || Ctrl->In.file == NULL, "Must specify a main script file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->Z.active, "Cannot use -Z if -Q is also set\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active || (Ctrl->N.prefix == NULL || strlen (Ctrl->N.prefix) == 0),
					"Option -N: Must specify a batch prefix\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active,
					"Option -T: Must specify number of jobs or a time file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !(Ctrl->Q.active || Ctrl->M.active),
					"Option -Z: Cannot be used without specifying a GIF (-A), master (-M) or batch (-F) product\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && Ctrl->M.job < Ctrl->T.start_job,
					"Option -M: Cannot specify a job before the first job number set via -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->W.active && Ctrl->W.dir && !strcmp (Ctrl->W.dir, "/tmp"),
					"Option -Z: Cannot delete working directory %s\n", Ctrl->W.dir);

	if (n_errors) return (GMT_PARSE_ERROR);	/* No point going further */

	/* Note: We open script files for reading below since we are changing cwd later and sometimes need to rewind and re-read */

	if (n_files == 1) {	/* Determine scripting language from file extension and open the main script file */
		if (strstr (Ctrl->In.file, ".bash") || strstr (Ctrl->In.file, ".sh"))	/* Treat both as bash since sh is subset of bash */
			Ctrl->In.mode = BASH_MODE;
		else if (strstr (Ctrl->In.file, ".csh"))
			Ctrl->In.mode = CSH_MODE;
		else if (strstr (Ctrl->In.file, ".bat"))
			Ctrl->In.mode = DOS_MODE;
		else {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to determine script language from the extension of your script %s\n", Ctrl->In.file);
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Allowable extensions are: *.sh, *.bash, *.csh, and *.bat\n");
			n_errors++;
		}
		/* Armed with script language we check that any back/fore-ground scripts are of the same kind */
		for (k = BATCH_PREFLIGHT; !n_errors && k <= BATCH_POSTFLIGHT; k++) {
			if (!Ctrl->S[k].active) continue;	/* Not provided */
			n_errors += batch_check_language (GMT, Ctrl->In.mode, Ctrl->S[k].file, k);
		}
		if (!n_errors && Ctrl->I.active) {	/* Must also check the include file, and open it for reading */
			n_errors += batch_check_language (GMT, Ctrl->In.mode, Ctrl->I.file, 3);
			if (n_errors == 0 && ((Ctrl->I.fp = fopen (Ctrl->I.file, "r")) == NULL)) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to open include script file %s\n", Ctrl->I.file);
				n_errors++;
			}
		}
		/* Open the main script for reading here */
		if (n_errors == 0 && ((Ctrl->In.fp = fopen (Ctrl->In.file, "r")) == NULL)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to open main script file %s\n", Ctrl->In.file);
			n_errors++;
		}
		if (n_errors == 0 && batch_script_is_classic (GMT, Ctrl->In.fp)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Your main script file %s is not in GMT modern mode\n", Ctrl->In.file);
			n_errors++;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void batch_close_files (struct BATCH_CTRL *Ctrl) {
	/* Close all files when an error forces us to quit */
	fclose (Ctrl->In.fp);
	if (Ctrl->I.active) fclose (Ctrl->I.fp);
}

GMT_LOCAL bool batch_line_is_a_comment (char *line) {
	unsigned int k = 0;
	while (line[k] && isspace (line[k])) k++;	/* Wind past leading whitespace */
	return (line[k] == '#' || !strncasecmp (&line[k], "rem", 3U)) ? true : false;	/* Will return true for lines starting with some tabs, then comment */
}

GMT_LOCAL bool batch_is_gmt_module (char *line, char *module) {
	/* Robustly identify the command "gmt begin" */
	char word[GMT_LEN128] = {""};
	unsigned int pos = 0;
	size_t L;
	if (strlen (line) >= GMT_LEN128) return false;	/* Cannot be gmt begin */
	/* To handle cases where there may be more than one space between gmt and module */
	if (batch_line_is_a_comment (line)) return false;	/* Skip commented lines like "  # anything" */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get first word in the command or fail */
	if (strcmp (word, "gmt")) return false;		/* Not starting with gmt so we are done */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get second word or fail */
	L = strlen (module);				/* How many characters to compare against */
	if (!strncmp (word, module, L)) return true;	/* Command starting with gmt <module> found */
	return false;	/* Not gmt <module> */
}

GMT_LOCAL bool batch_is_gmt_end_show (char *line) {
	char word[GMT_LEN128] = {""};
	unsigned int pos = 0;
	if (strlen (line) >= GMT_LEN128) return false;	/* Cannot be gmt end show */
	/* Robustly identify the command "gmt end show" */
	/* To handle cases where there may be more than one space between gmt and module */
	if (batch_line_is_a_comment (line)) return false;	/* Skip commented lines like "  # anything" */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get first word in the command or fail */
	if (strcmp (word, "gmt")) return false;		/* Not starting with gmt so we are done */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get second word or fail */
	if (strcmp (word, "end")) return false;		/* Not continuing with end so we are done */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get third word or fail */
	if (!strcmp (word, "show")) return true;	/* Yes, found gmt end show */
	return false;	/* Not gmt end show */
}

GMT_LOCAL int batch_delete_scripts (struct GMT_CTRL *GMT, struct BATCH_CTRL *Ctrl) {
	/* Delete the scripts since they apparently are temporary */
	if (Ctrl->In.file && gmt_remove_file (GMT, Ctrl->In.file)) {	/* Delete the main script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the main script %s.\n", Ctrl->In.file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->I.file && gmt_remove_file (GMT, Ctrl->I.file)) {	/* Delete the include script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the include script %s.\n", Ctrl->I.file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->S[BATCH_PREFLIGHT].file && gmt_remove_file (GMT, Ctrl->S[BATCH_PREFLIGHT].file)) {	/* Delete the post-run script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the post-run script %s.\n", Ctrl->S[BATCH_PREFLIGHT].file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->S[BATCH_POSTFLIGHT].file && gmt_remove_file (GMT, Ctrl->S[BATCH_POSTFLIGHT].file)) {	/* Delete the preflight script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the preflight script %s.\n", Ctrl->S[BATCH_POSTFLIGHT].file);
		return (GMT_RUNTIME_ERROR);
	}
	return (GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_batch (void *V_API, int mode, void *args) {
	int error = 0, precision, scol, srow;
	int (*run_script)(const char *);	/* pointer to system function or a dummy */

	unsigned int n_values = 0, n_jobs = 0, n_data_jobs, job, i_job, col, k, T;
	unsigned int n_jobs_not_started = 0, n_jobs_completed = 0, first_job = 0, data_job, n_cores_unused;
	unsigned int dd, hh, mm, ss, start, flavor[2] = {0, 0};

	bool done = false,  one_job = false, upper_case[2] = {false, false};
	bool n_written = false, has_text = false, is_classic = false;

	char *extension[3] = {"sh", "csh", "bat"}, *load[3] = {"source", "source", "call"}, *rmfile[3] = {"rm -f", "rm -f", "del"};
	char *rmdir[3] = {"rm -rf", "rm -rf", "rd /s /q"}, *export[3] = {"export ", "setenv ", ""};
	char *mvfile[3] = {"mv -f", "mv -f", "move /Y"}, *sc_call[3] = {"bash ", "csh ", "start /B"};
	char *createfile[3] = {"touch", "touch", "copy /b NUL"};
	char var_token[4] = "$$%", spacer;
	char init_file[PATH_MAX] = {""}, state_tag[GMT_LEN16] = {""}, state_prefix[GMT_LEN64] = {""}, param_file[PATH_MAX] = {""}, cwd[PATH_MAX] = {""};
	char pre_file[PATH_MAX] = {""}, post_file[PATH_MAX] = {""}, main_file[PATH_MAX] = {""}, line[PATH_MAX] = {""}, version[GMT_LEN32] = {""};
	char string[GMT_LEN128] = {""}, extra[GMT_LEN256] = {""}, cmd[GMT_LEN256] = {""}, cleanup_file[PATH_MAX] = {""}, L_txt[GMT_LEN128] = {""};
	char completion_file[PATH_MAX] = {""}, topdir[PATH_MAX] = {""}, workdir[PATH_MAX] = {""}, datadir[PATH_MAX] = {""}, job_products[GMT_LEN32] = {""};
	char intro_file[PATH_MAX] = {""}, *script_file =  NULL, dir_sep = '/', which[2] = {"LP"};
	double percent = 0.0, L_col = 0, sx, sy, fade_level = 0.0;

	FILE *fp = NULL;

	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_OPTION *options = NULL;
	struct BATCH_STATUS *status = NULL;
	struct BATCH_CTRL *Ctrl = NULL;
	struct BATCH_ITEM *I = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the batch main code ----------------------------*/

	if (Ctrl->Q.scripts) {	/* No batch, but scripts will be produced */
		GMT_Report (API, GMT_MSG_INFORMATION, "Dry-run enabled - Movie scripts will be created and any pre/post scripts will be executed.\n");
		if (Ctrl->M.active) GMT_Report (API, GMT_MSG_INFORMATION, "A single script for job %d will be created and executed\n", Ctrl->M.job);
		run_script = batch_dry_run_only;	/* This prevents the main job loop from executing the script */
	}
	else	/* Will run scripts and may even need to make a batch */
		run_script = system;	/* The standard system function will be used */

	/* First try to read -T<timefile> in case it is prescribed directly (and before we change directory) */
	if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name exists and is readable */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
			batch_close_files (Ctrl);
			Return (API->error);
		}
		if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
			GMT_Report (API, GMT_MSG_ERROR, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
			batch_close_files (Ctrl);
			Return (API->error);
		}
		n_jobs = (unsigned int)D->n_records;	/* Number of records means number of jobs */
		n_values = (unsigned int)D->n_columns;	/* The number of per-job parameters we need to place into the per-job parameter files */
		has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		if (n_jobs == 0) {	/* So not good... */
			GMT_Report (API, GMT_MSG_ERROR, "Your time file %s has no records - exiting\n", Ctrl->T.file);
			batch_close_files (Ctrl);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	n_data_jobs = n_jobs;

	if (Ctrl->W.active) {
		if (Ctrl->W.dir)
			strcpy (workdir, Ctrl->W.dir);
		else 	/* Make one in tempdir based on N.prefix */
			sprintf (workdir, "%s/%s", API->tmp_dir, Ctrl->N.prefix);
	}
	else
		strcpy (workdir, Ctrl->N.prefix);

	/* Get full path to the current working directory */
	if (getcwd (topdir, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to determine current working directory - exiting.\n");
		batch_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	gmt_replace_backslash_in_path (topdir);

	/* Create a working directory which will house every local file and all subdirectories created */
	if (gmt_mkdir (workdir)) {
		GMT_Report (API, GMT_MSG_ERROR, "An old directory named %s exists OR we were unable to create new working directory %s - exiting.\n", workdir, workdir);
		batch_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Make this directory the current working directory */
	if (chdir (workdir)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to change directory to %s - exiting.\n", workdir);
		perror (workdir);
		batch_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Get full path to this working directory */
	if (getcwd (cwd, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to determine current working directory.\n");
		batch_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}

	/* We use DATADIR to include the top and working directory so any files we supply or create can be found while inside job directory */

	if (GMT->session.DATADIR)	/* Prepend initial and subdir as new datadirs to the existing search list */
		sprintf (datadir, "%s,%s,%s", topdir, cwd, GMT->session.DATADIR);	/* Start with topdir */
	else	/* Set the initial and prefix subdirectory as data dirs */
		sprintf (datadir, "%s,%s", topdir, cwd);

	gmt_replace_backslash_in_path (datadir);	/* Since we will be fprintf the path we must use // for a slash */
	gmt_replace_backslash_in_path (workdir);

	/* Create the initialization file with settings common to all jobs */

	n_written = (n_jobs > 0);
	sprintf (init_file, "batch_init.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_INFORMATION, "Create parameter initiation script %s\n", init_file);
	if ((fp = fopen (init_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s - exiting\n", init_file);
		batch_close_files (Ctrl);
		Return (GMT_ERROR_ON_FOPEN);
	}

	sprintf (string, "Static parameters set for processing sequence %s", Ctrl->N.prefix);
	batch_set_comment (fp, Ctrl->In.mode, string);
	if (n_written) batch_set_ivalue (fp, Ctrl->In.mode, false, "BATCH_NJOBS", n_data_jobs);	/* Total jobs (write to init since known) */
	if (Ctrl->I.active) {	/* Append contents of an include file */
		batch_set_comment (fp, Ctrl->In.mode, "Static parameters set via user include file");
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->I.fp)) {	/* Read the include file and copy to init script with some exceptions */
			if (batch_is_gmt_module (line, "begin")) continue;		/* Skip gmt begin */
			if (batch_is_gmt_module (line, "end")) continue;		/* Skip gmt end */
			if (strstr (line, "#!/")) continue;			/* Skip any leading shell incantation */
			if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
			fprintf (fp, "%s", line);				/* Just copy the line as is */
		}
		fclose (Ctrl->I.fp);	/* Done reading the include script */
	}
	fclose (fp);	/* Done writing the init script */

	if (Ctrl->S[BATCH_PREFLIGHT].active) {	/* Create the preflight script from the user's post-run script */
		/* The post-run script must be modern mode */
		unsigned int rec = 0;
		sprintf (pre_file, "batch_preflight.%s", extension[Ctrl->In.mode]);
		is_classic = batch_script_is_classic (GMT, Ctrl->S[BATCH_PREFLIGHT].fp);
		if (is_classic) {
			GMT_Report (API, GMT_MSG_ERROR, "Your preflight file %s is not in GMT modern node - exiting\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Create preflight script %s and execute it\n", pre_file);
		if ((fp = fopen (pre_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create preflight script %s - exiting\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		batch_set_script (fp, Ctrl->In.mode);			/* Write 1st line of a script */
		batch_set_comment (fp, Ctrl->In.mode, "Preflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);		/* Hardwire a Session Name since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately first */
			fprintf (fp, "set GMT_SESSION_NAME=1\n");
		else	/* On UNIX we may use the calling terminal or script's PID as the GMT_SESSION_NAME */
			batch_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		fprintf (fp, "%s", export[Ctrl->In.mode]);		/* Turn off auto-display of figures if scrip has gmt end show */
		batch_set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[BATCH_PREFLIGHT].fp)) {	/* Read the post-run script and copy to preflight script with some exceptions */
			if (batch_is_gmt_module (line, "begin")) {
				fprintf (fp, "gmt begin\n");	/* To ensure there are no args here since we are using gmt figure instead */
				fprintf (fp, "\tgmt set DIR_DATA %s\n", datadir);
			}
			else if (!strstr (line, "#!/"))	 {	/* Skip any leading shell incantation since already placed by batch_set_script */
				if (batch_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
				else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			}
			rec++;
		}
		fclose (Ctrl->S[BATCH_PREFLIGHT].fp);	/* Done reading the preflight script */
		fclose (fp);	/* Done writing the preflight script */
#ifndef WIN32
		/* Set executable bit if not on Windows */
		if (chmod (pre_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to make preflight script %s executable - exiting.\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
		/* Run the pre-flight now which may or may not create a <timefile> needed later via -T */
		if (Ctrl->In.mode == DOS_MODE)	/* Needs to be "cmd /C" and not "start /B" to let it have time to finish */
			sprintf (cmd, "cmd /C %s", pre_file);
		else
			sprintf (cmd, "%s %s", sc_call[Ctrl->In.mode], pre_file);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running preflight script %s returned error %d - exiting.\n", pre_file, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Now we can complete the -T parsing since any given file has now been created by pre_file */

	if (n_jobs == 0) {	/* Must check again for a file or decode the argument as a number */
		if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name was indeed created by preflight and is now available */
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
				fclose (Ctrl->In.fp);
				Return (API->error);
			}
			if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
				GMT_Report (API, GMT_MSG_ERROR, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
				fclose (Ctrl->In.fp);
				Return (API->error);
			}
			n_jobs = n_data_jobs = (unsigned int)D->n_records;	/* Number of records means number of jobs */
			n_values = (unsigned int)D->n_columns;	/* The number of per-job parameters we need to place into the per-job parameter files */
			has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		}
		else if (gmt_count_char (GMT, Ctrl->T.file, '/') == 2) {	/* Give a vector specification -Tmin/max/inc, call gmtmath */
			char output[GMT_VF_LEN] = {""}, cmd[GMT_LEN128] = {""};
			unsigned int V = GMT->current.setting.verbose;
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, NULL, output) == GMT_NOTSET) {
				Return (API->error);
			}
			if (GMT->common.f.active[GMT_IN])
				sprintf (cmd, "-T%s -o1 -f%s --GMT_HISTORY=false T = %s", Ctrl->T.file, GMT->common.f.string, output);
			else
				sprintf (cmd, "-T%s -o1 --GMT_HISTORY=false T = %s", Ctrl->T.file, output);
			GMT_Report (API, GMT_MSG_INFORMATION, "Calling gmtmath with args %s\n", cmd);
			GMT->current.setting.verbose = GMT_MSG_ERROR;	/* So we don't get unwanted verbosity from gmtmath */
  			if (GMT_Call_Module (API, "gmtmath", GMT_MODULE_CMD, cmd)) {
				Return (API->error);	/* Some sort of failure */
			}
			GMT->current.setting.verbose = V;	/* Restore */
			if ((D = GMT_Read_VirtualFile (API, output)) == NULL) {	/* Load in the data array */
				Return (API->error);	/* Some sort of failure */
			}
			n_jobs = n_data_jobs = (unsigned int)D->n_records;	/* Number of records means number of jobs */
			n_values = (unsigned int)D->n_columns;	/* The number of per-job parameters we need to place into the per-job parameter files */
			has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		}
		else	/* Just gave the number of jobs (we hope, or we got a bad filename and atoi should return 0) */
			n_jobs = n_data_jobs = atoi (Ctrl->T.file);
	}
	if (n_jobs == 0) {	/* So not good... */
		GMT_Report (API, GMT_MSG_ERROR, "No jobs specified! - exiting.\n");
		fclose (Ctrl->In.fp);
		Return (GMT_RUNTIME_ERROR);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of main processing jobs: %d\n", n_data_jobs);
	if (Ctrl->T.precision)	/* Precision was prescribed */
		precision = Ctrl->T.precision;
	else	/* Compute width from largest job number */
		precision = irint (ceil (log10 ((double)(Ctrl->T.start_job+n_jobs))));	/* Width needed to hold largest job number */

	if (Ctrl->S[BATCH_POSTFLIGHT].active) {	/* Prepare the postflight script */
		sprintf (post_file, "batch_postflight.%s", extension[Ctrl->In.mode]);
		is_classic = batch_script_is_classic (GMT, Ctrl->S[BATCH_POSTFLIGHT].fp);
		if (is_classic) {
			GMT_Report (API, GMT_MSG_ERROR, "Your postflight file %s is not in GMT modern node - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Create postflight script %s\n", post_file);
		if ((fp = fopen (post_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create postflight file %s - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		batch_set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		batch_set_comment (fp, Ctrl->In.mode, "Postflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a SESSION_NAME since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately */
			fprintf (fp, "set GMT_SESSION_NAME=1\n");
		else	/* On UNIX we may use the script's PID as GMT_SESSION_NAME */
			batch_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Turn off auto-display of figures if scrip has gmt end show */
		batch_set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[BATCH_POSTFLIGHT].fp)) {	/* Read the preflight script and copy to postflight script with some exceptions */
			if (batch_is_gmt_module (line, "begin")) {	/* Need to insert gmt figure after this line */
				fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
				fprintf (fp, "\tgmt set DIR_DATA %s\n", datadir);
			}
			else if (!strstr (line, "#!/"))	{	/* Skip any leading shell incantation since already placed */
				if (batch_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
				else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			}
		}
		fclose (Ctrl->S[BATCH_POSTFLIGHT].fp);	/* Done reading the preflight script */
		fclose (fp);	/* Done writing the postflight script */
#ifndef WIN32
		/* Set executable bit if not Windows */
		if (chmod (post_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to make postflight script %s executable - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
		/* Run post-flight now before dealing with the loop so the overlay exists */
		if (Ctrl->In.mode == DOS_MODE)	/* Needs to be "cmd /C" and not "start /B" to let it have time to finish */
			sprintf (cmd, "cmd /C %s", post_file);
		else
			sprintf (cmd, "%s %s", sc_call[Ctrl->In.mode], post_file);
		if ((error = run_script (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running postflight script %s returned error %d - exiting.\n", post_file, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}

#ifndef WIN32
		/* Set executable bit if not Windows */
		if (chmod (intro_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to make script %s executable - exiting\n", intro_file);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
	}

	/* Create parameter include files, one for each job */

	GMT_Report (API, GMT_MSG_INFORMATION, "Parameter files for main processing: %d\n", n_jobs);
	for (i_job = 0; i_job < n_jobs; i_job++) {
		job = data_job = i_job + Ctrl->T.start_job;	/* The job is normally same as data_job number */
		if (one_job && job != Ctrl->M.job) continue;	/* Just doing a single job for debugging */
		sprintf (state_tag, "%*.*d", precision, precision, job);
		sprintf (state_prefix, "batch_params_%s", state_tag);
		sprintf (param_file, "%s.%s", state_prefix, extension[Ctrl->In.mode]);
		if ((fp = fopen (param_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create job parameter file %s - exiting\n", param_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		sprintf (state_prefix, "Parameter file for job %s", state_tag);
		batch_set_comment (fp, Ctrl->In.mode, state_prefix);
		sprintf (state_prefix, "%s_%s", Ctrl->N.prefix, state_tag);
		batch_set_tvalue (fp, Ctrl->In.mode, false, "BATCH_NAME", state_prefix);	/* Current job name prefix (e.g., my_job_0003) */
		batch_set_ivalue (fp, Ctrl->In.mode, false, "BATCH_JOB", data_job);		/* Current job number (e.g., 3) */
		if (!n_written) batch_set_ivalue (fp, Ctrl->In.mode, false, "BATCH_NJOBS", n_data_jobs);	/* Total jobs (write here since n_jobs was not yet known when init was written) */
		batch_set_tvalue (fp, Ctrl->In.mode, false, "BATCH_ITEM", state_tag);		/* Current job tag (formatted job number, e.g, 0003) */
		for (col = 0; col < n_values; col++) {	/* Derive job variables from <timefile> in each parameter file */
			sprintf (string, "BATCH_COL%u", col);
			batch_set_value (GMT, fp, Ctrl->In.mode, col, string, D->table[0]->segment[0]->data[col][data_job]);
		}
		if (has_text) {	/* Also place any string parameter as a single string variable */
			batch_set_tvalue (fp, Ctrl->In.mode, false, "BATCH_TEXT", D->table[0]->segment[0]->text[data_job]);
			if (Ctrl->T.split) {	/* Also split the string into individual words BATCH_WORD1, BATCH_WORD2, etc. */
				char *word = NULL, *trail = NULL, *orig = strdup (D->table[0]->segment[0]->text[data_job]);
				col = 0;
				trail = orig;
				while ((word = strsep (&trail, " \t")) != NULL) {
					if (*word != '\0') {	/* Skip empty strings */
						sprintf (string, "BATCH_WORD%u", col++);
						batch_set_tvalue (fp, Ctrl->In.mode, false, string, word);
					}
				}
				gmt_M_str_free (orig);
			}
		}
		fclose (fp);	/* Done writing this parameter file */
	}

	/* Now build the main loop script from the mainscript */

	sprintf (main_file, "batch_job.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_INFORMATION, "Create main batch job script %s\n", main_file);
	if ((fp = fopen (main_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create loop job script file %s - exiting\n", main_file);
		fclose (Ctrl->In.fp);
		Return (GMT_ERROR_ON_FOPEN);
	}
	batch_set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
	batch_set_comment (fp, Ctrl->In.mode, "Main job loop script");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a GMT_SESSION_NAME since subshells may mess things up */
	if (Ctrl->In.mode == DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to be the job number */
		fprintf (fp, "set GMT_SESSION_NAME=%c1\n", var_token[Ctrl->In.mode]);
	else	/* On UNIX we use the script's PID as GMT_SESSION_NAME */
		batch_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Turn off auto-display of figures if script has gmt end show */
	batch_set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
	batch_set_comment (fp, Ctrl->In.mode, "Include static and job-specific parameters");
	fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
	fprintf (fp, "%s batch_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the job parameters */
	fprintf (fp, "mkdir %s\n", batch_place_var (Ctrl->In.mode, "BATCH_NAME"));	/* Make a temp directory for this job */
	fprintf (fp, "cd %s\n", batch_place_var (Ctrl->In.mode, "BATCH_NAME"));		/* cd to the temp directory */
	while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the main script and copy to loop script, with some exceptions */
		if (batch_is_gmt_module (line, "begin")) {
			fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are not building plots */
		}
		else if (!strstr (line, "#!/")) {		/* Skip any leading shell incantation since already placed */
			if (batch_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
			else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
			fprintf (fp, "%s", line);	/* Just copy the line as is */
		}
	}
	fclose (Ctrl->In.fp);	/* Done reading the main script */
	/* Move job products up to main directory */
	fprintf (fp, "%s %s.* ..\n", mvfile[Ctrl->In.mode], batch_place_var (Ctrl->In.mode, "BATCH_NAME"));
	fprintf (fp, "cd ..\n");	/* cd up to parent dir */
	/* Create completion file */
	fprintf (fp, "%s %s.___\n", createfile[Ctrl->In.mode], batch_place_var (Ctrl->In.mode, "BATCH_NAME"));
	if (!Ctrl->Q.active) {	/* Delete evidence; otherwise we want to leave debug evidence when doing a single job only */
		batch_set_comment (fp, Ctrl->In.mode, "Remove job directory and job parameter file");
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], batch_place_var (Ctrl->In.mode, "BATCH_NAME"));	/* Remove the work dir and any files in it */
		fprintf (fp, "%s batch_params_%c1.%s\n", rmfile[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Remove the parameter file for this job */
	}
	if (Ctrl->In.mode == DOS_MODE)	/* This is crucial to the "start /B ..." statement below to ensure the DOS process terminates */
		fprintf (fp, "exit\n");
	fclose (fp);	/* Done writing loop script */

#ifndef WIN32
	/* Set executable bit if not Windows */
	if (chmod (main_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to make script %s executable - exiting\n", main_file);
		Return (GMT_RUNTIME_ERROR);
	}
#endif

	GMT_Report (API, GMT_MSG_INFORMATION, "Total jobs to process: %u\n", n_jobs);

	if (Ctrl->Q.scripts) {	/* No processing executed */
		Return (GMT_NOERROR);	/* We are done */
	}

	/* Finally, we can run all the jobs in a controlled loop, launching new parallel jobs as cores become available */

	i_job = first_job = 0; n_jobs_not_started = n_jobs;
	job = Ctrl->T.start_job;
	n_cores_unused = MAX (1, Ctrl->x.n_threads - 1);			/* Remove one for the main batch module thread */
	status = gmt_M_memory (GMT, NULL, n_jobs, struct BATCH_STATUS);	/* Used to keep track of job status */
	script_file = main_file;
	GMT_Report (API, GMT_MSG_INFORMATION, "Build jobs using %u cores\n", n_cores_unused);
	/* START PARALLEL EXECUTION OF JOB SCRIPTS */
	GMT_Report (API, GMT_MSG_INFORMATION, "Execute batch job scripts in parallel\n");
	while (!done) {	/* Keep running jobs until all jobs have completed */
		while (n_jobs_not_started && n_cores_unused) {	/* Launch new jobs if possible */
#ifdef WIN32
			if (Ctrl->In.mode < 2)		/* A bash or sh run from Windows. Need to call via "start" to get parallel */
				sprintf (cmd, "start /B %s %s %*.*d", sc_call[Ctrl->In.mode], script_file, precision, precision, job);
			else						/* Running batch, so no need for the above trick */
				sprintf (cmd, "%s %s %*.*d &", sc_call[Ctrl->In.mode], script_file, precision, precision, job);
#else
			sprintf (cmd, "%s %s %*.*d &", sc_call[Ctrl->In.mode], script_file, precision, precision, job);
#endif

			GMT_Report (API, GMT_MSG_DEBUG, "Launch script for job %*.*d\n", precision, precision, job);
			if ((error = system (cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Running script %s returned error %d - aborting.\n", cmd, error);
				Return (GMT_RUNTIME_ERROR);
			}
			status[job].started = true;	/* We have now launched this job job */
			job++;			/* Advance to next job for next launch */
			i_job++;			/* Advance to next job for next launch */
			n_jobs_not_started--;		/* One less job remaining */
			n_cores_unused--;		/* This core is now busy */
		}
		batch_gmt_sleep (BATCH_WAIT_TO_CHECK);	/* Wait 0.01 second - then check for completion of the completion file */
		for (k = first_job; k < i_job; k++) {	/* Only loop over the range of jobs that we know are currently in play */
			if (status[k].completed) continue;	/* Already finished with this job */
			if (!status[k].started) continue;	/* Not started this job yet */
			/* Here we can check if the job job has completed by looking for the completion file */
			sprintf (completion_file, "%s_%*.*d.___", Ctrl->N.prefix, precision, precision, Ctrl->T.start_job+k);
			if (access (completion_file, F_OK)) continue;	/* Not found yet */
			n_jobs_completed++;		/* One more job completed */
			status[k].completed = true;	/* Flag this job as completed */
			n_cores_unused++;		/* Free up the core */
			percent = 100.0 * n_jobs_completed / n_jobs;
			GMT_Report (API, GMT_MSG_INFORMATION, "Frame %*.*d of %d completed [%5.1f %%]\n", precision, precision, k, n_jobs, percent);
		}
		/* Adjust first_job, if needed */
		while (first_job < n_jobs && status[first_job].completed) first_job++;
		if (n_jobs_completed == n_jobs) done = true;	/* All jobs completed! */
	}
	/* END PARALLEL EXECUTION OF JOB SCRIPTS */

	gmt_M_free (GMT, status);	/* Done with this structure array */

	/* Cd back up to the parent directory */
	if (chdir (topdir)) {	/* Should never happen but we should check */
		GMT_Report (API, GMT_MSG_ERROR, "Unable to change directory to starting directory - exiting.\n");
		perror (topdir);
		Return (GMT_RUNTIME_ERROR);
	}

	/* Prepare the cleanup script */
	sprintf (cleanup_file, "batch_cleanup.%s", extension[Ctrl->In.mode]);
	if ((fp = fopen (cleanup_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create cleanup file %s - exiting\n", cleanup_file);
		Return (GMT_ERROR_ON_FOPEN);
	}
	batch_set_script (fp, Ctrl->In.mode);		/* Write 1st line of a script */
	if (Ctrl->Z.active) {	/* Want to delete the entire job directory */
		batch_set_comment (fp, Ctrl->In.mode, "Cleanup script removes working directory with job files");
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], workdir);	/* Delete the entire working directory with batch jobs and tmp files */
	}
	else {	/* Just delete the remaining scripts and PS files */
#ifdef WIN32		/* On Windows to do remove a file in a subdir one need to use back slashes */
		char dir_sep_ = '\\';
#else
		char dir_sep_ = '/';
#endif
		GMT_Report (API, GMT_MSG_INFORMATION, "%u job files saved in directory: %s\n", n_jobs, workdir);
		if (Ctrl->S[BATCH_PREFLIGHT].active)	/* Remove the preflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, pre_file);
		if (Ctrl->S[BATCH_POSTFLIGHT].active)	/* Remove the postflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, post_file);
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, init_file);	/* Delete the init script */
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, main_file);	/* Delete the main script */
	}
	fclose (fp);
#ifndef _WIN32
	/* Set executable bit if not on Windows */
	if (chmod (cleanup_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to make cleanup script %s executable - exiting\n", cleanup_file);
		Return (GMT_RUNTIME_ERROR);
	}
#endif
	if (!Ctrl->Q.active) {
		/* Run cleanup script at the end */
		if (Ctrl->In.mode == DOS_MODE)
			error = system (cleanup_file);
		else {
			sprintf (cmd, "%s %s", sc_call[Ctrl->In.mode], cleanup_file);
			error = system (cmd);
		}
		if (error) {
			GMT_Report (API, GMT_MSG_ERROR, "Running cleanup script %s returned error %d - exiting.\n", cleanup_file, error);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->Z.delete) {	/* Delete input scripts */
		if ((error = batch_delete_scripts (GMT, Ctrl)))
			Return (error);
	}

	/* Finally, delete the clean-up script separately since under DOS we got complaints when we had it delete itself (which works under *nix) */
	if (!Ctrl->Q.active && gmt_remove_file (GMT, cleanup_file)) {	/* Delete the cleanup script itself */
		GMT_Report (API, GMT_MSG_ERROR, "Unable to delete the cleanup script %s.\n", cleanup_file);
		Return (GMT_RUNTIME_ERROR);
	}

	Return (GMT_NOERROR);
}
