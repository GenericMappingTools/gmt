/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Date:	12-May-2020
 * Version:	6 API
 *
 * Brief synopsis: gmt batch automates batch processing
 *
 * batch automates the main processing loop and much of the machinery needed
 * to script a batch job sequence.  It allows for optional preflight and
 * and postflight scripts to be specified via separate modern mode scripts and
 * a single processing script that uses special variables to use slightly
 * different parameters for each job.  The user only needs to compose these
 * simple one-job scripts and then batch takes care of the automation.
 * Jobs are run in parallel without need for OpenMP, etc.
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

/* Control structure for batch */

struct BATCH_CTRL {
	struct BATCH_In {	/* mainscript (Bourne, Bourne Again, csh, or DOS (bat) script) */
		bool active;
		enum GMT_enum_script mode;
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
	struct BATCH_T {	/* -T<n_jobs>|<min>/<max/<inc>[+n]|<timefile>[+p<precision>][+s<job>][+w[<str>]] */
		bool active;
		bool split;		/* true means we must split any trailing text in to words, using separators in <str> [" \t"] */
		unsigned int n_jobs;	/* Total number of jobs */
		unsigned int start_job;	/* First job [0] */
		unsigned int precision;	/* Decimals used in making unique job tags */
		char sep[GMT_LEN8];		/* word separator(s) */
		char *file;		/* timefile name */
	} T;
	struct BATCH_W {	/* -W<workingdirectory> */
		bool active;
		char *dir;	/* Alternative working directory than implied by -N */
	} W;
	struct BATCH_Z {	/* -Z[ */
		bool active;	//* Delete all files including the mainscript and anything passed via -I, -S */
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
	strcpy (C->T.sep, "\t ");	/* Any white space */
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

GMT_LOCAL void batch_close_files (struct BATCH_CTRL *Ctrl) {
	/* Close all files when an error forces us to quit */
	fclose (Ctrl->In.fp);
	if (Ctrl->I.active) fclose (Ctrl->I.fp);
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
	if (Ctrl->S[BATCH_PREFLIGHT].file && gmt_remove_file (GMT, Ctrl->S[BATCH_PREFLIGHT].file)) {	/* Delete the background script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the preflight script %s.\n", Ctrl->S[BATCH_PREFLIGHT].file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->S[BATCH_POSTFLIGHT].file && gmt_remove_file (GMT, Ctrl->S[BATCH_POSTFLIGHT].file)) {	/* Delete the foreground script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the postflight script %s.\n", Ctrl->S[BATCH_POSTFLIGHT].file);
		return (GMT_RUNTIME_ERROR);
	}
	return (GMT_NOERROR);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <mainscript> -N<prefix> -T<njobs>|<min>/<max>/<inc>[+n]|<timefile>[+p<width>][+s<first>][+w[<str>]|W] "
		"[-I<includefile>] [-M[<job>]] [-Q[s]] [-Sb<postflight>] [-Sf<preflight>] "
		"[%s] [-W[<dir>]] [-Z] [%s] [-x[[-]<n>]] [%s]\n", name, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<mainscript>");
	GMT_Usage (API, -2, "The main GMT modern script that completes a single job.");
	GMT_Usage (API, 1, "\n-N<prefix>");
	GMT_Usage (API, -2, "Set the <prefix> used for batch files and directory names.");
	GMT_Usage (API, 1, "\n-T<njobs>|<min>/<max>/<inc>[+n]|<timefile>[+p<width>][+s<first>][+w[<str>]|W]");
	GMT_Usage (API, -2, "Set number of jobs, create parameters from <min>/<max>/<inc>[+n] or give file with job-specific information. "
		"If <timefile> does not exist it must be created by the preflight script given via -Sf.");
	GMT_Usage (API, 3, "+p Set number of digits used in creating the job tags [automatic].");
	GMT_Usage (API, 3, "+n Indicate that <inc> in <min>/<max>/<inc> is in fact number of jobs instead.");
	GMT_Usage (API, 3, "+s Append <first> to change the value of the first job [0].");
	GMT_Usage (API, 3, "+w Let trailing text in <timefile> be split into individual word variables. "
		"We use space or TAB as separators; append <str> to set custom characters as separators instead.");
	GMT_Usage (API, 3, "+W Same as +w but only use TAB as separator.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-I<includefile>");
	GMT_Usage (API, -2, "Specify a script file to be inserted into the batch_init.sh script [none]. "
		"Used to add constant variables needed by all batch scripts.");
	GMT_Usage (API, 1, "\n-M[<job>]");
	GMT_Usage (API, -2, "Run just the indicated job number [0] for testing [run all].");
	GMT_Usage (API, 1, "\n-Q[s]");
	GMT_Usage (API, -2, "Debugging: Leave all intermediate files and directories behind for inspection. "
		"Append s to only create the work scripts but none will be executed (except for <preflight> script).");
	GMT_Usage (API, 1, "\n-Sf<preflight>");
	GMT_Usage (API, -2, "Append name of <preflight> GMT modern script that may download or compute "
		"files needed by the <mainscript>.");
	GMT_Usage (API, 1, "\n-Sb<postflight>");
	GMT_Usage (API, -2, "Append name of <postflight> script (not necessarily a GMT script) which will "
		"take actions once all batch jobs have completed.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W[<dir>]");
	GMT_Usage (API, -2, "Specify <dir> where temporary files will be built [<dir> = <prefix> set by -N]. "
		"If <dir> is not given we create one in the system temp directory named <prefix> (from -N).");
	GMT_Usage (API, 1, "\n-Z");
	GMT_Usage (API, -2, "Erase input scripts (<mainscript> and any files via -I, -S) [leave input scripts alone]. Not compatible with -Q.");
	GMT_Option (API, "f");
	/* Number of threads (re-purposed from -x in GMT_Option since this local option is always available and we are not using OpenMP) */
	GMT_Usage (API, 1, "\n-x[[-]<n>]");
	GMT_Usage (API, -2, "Limit the number of cores used in job generation [Default uses all %d cores]. "
		"If <n> is negative then we select (%d - <n>) cores (or at least 1). Note: 1 core is used by batch itself.", API->n_cores, API->n_cores);
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct BATCH_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to batch and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 */

	unsigned int n_errors = 0, n_files = 0, k, pos;
	char p[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file */
				if (n_files++ > 0) break;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file)))
					n_errors++;
				else
					Ctrl->In.active = true;
				break;

			case 'I':	/* Include file with settings used by all scripts */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				Ctrl->I.file = strdup (opt->arg);
				break;

			case 'M':	/* Create a single job as well as batch (unless -Q is active) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.active = true;
				if (opt->arg[0])	/* Gave a job number */
					Ctrl->M.job = atoi (opt->arg);
				break;

			case 'N':	/* Movie prefix and directory name */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				Ctrl->N.prefix = strdup (opt->arg);
				break;

			case 'Q':	/* Debug - leave temp files and directories behind; Use -Qs to only write scripts */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
				if (opt->arg[0] == 's') Ctrl->Q.scripts = true;
				break;

			case 'S':	/* postflight and preflight scripts */
				if (opt->arg[0] == 'b')
					k = BATCH_PREFLIGHT;	/* postflight */
				else if (opt->arg[0] == 'f')
					k = BATCH_POSTFLIGHT;	/* preflight */
				else {	/* Bad option */
					n_errors++;
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Select -Sb or -Sf\n");
					break;
				}
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S[k].active);
				/* Got a valid f or b */
				Ctrl->S[k].active = true;
				Ctrl->S[k].file = strdup (&opt->arg[1]);
				if ((Ctrl->S[k].fp = fopen (Ctrl->S[k].file, "r")) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -S%c: Unable to open file %s\n", opt->arg[0], Ctrl->S[k].file);
					n_errors++;
				}
				break;

			case 'T':	/* Number of jobs or the name of file with job information (note: file may not exist yet) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
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
							case 'w':	/* Split trailing text into words using any white space. */
								Ctrl->T.split = true;
								if (p[1]) {	/* Gave an argument, watch out for tabs given as \t */
									char *W = gmt_get_strwithtab (&p[1]);
									strncpy (Ctrl->T.sep, W, GMT_LEN8-1);
									gmt_M_str_free (W);
								}
								break;
							case 'W':	/* Split trailing text into words using only TABs. */
								Ctrl->T.split = true;
								Ctrl->T.sep[1] = '\0';	/* Truncate the space character in the list to only have TAB */
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
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (opt->arg[0]) Ctrl->W.dir = strdup (opt->arg);
				break;

			case 'Z':	/* Delete input scripts */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
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
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active || (Ctrl->N.prefix == NULL || strlen (Ctrl->N.prefix) == 0),
					"Option -N: Must specify a batch prefix\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active,
					"Option -T: Must specify number of jobs or a time file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.split && Ctrl->T.sep[0] == '\0',
					"Option -T: Must specify a string of characters if using +w<str>\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->Z.active,
					"Option -Z: Not compatible with -Q\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && Ctrl->M.job < Ctrl->T.start_job,
					"Option -M: Cannot specify a job before the first job number set via -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.dir && !strcmp (Ctrl->W.dir, "/tmp"),
					"Option -W: Cannot delete working directory %s\n", Ctrl->W.dir);

	if (n_errors) return (GMT_PARSE_ERROR);	/* No point going further */

	/* Note: We open script files for reading below since we are changing cwd later and sometimes need to rewind and re-read */

	if (n_files == 1) {	/* Determine scripting language from file extension and open the main script file */
		if (strstr (Ctrl->In.file, ".bash") || strstr (Ctrl->In.file, ".sh"))	/* Treat both as bash since sh is subset of bash */
			Ctrl->In.mode = GMT_BASH_MODE;
		else if (strstr (Ctrl->In.file, ".csh"))
			Ctrl->In.mode = GMT_CSH_MODE;
		else if (strstr (Ctrl->In.file, ".bat"))
			Ctrl->In.mode = GMT_DOS_MODE;
		else {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to determine script language from the extension of your script %s\n", Ctrl->In.file);
			GMT_Report (API, GMT_MSG_ERROR, "Allowable extensions are: *.sh, *.bash, *.csh, and *.bat\n");
			n_errors++;
		}
		/* Armed with script language we check that any back/fore-ground scripts are of the same kind */
		for (k = BATCH_PREFLIGHT; !n_errors && k <= BATCH_POSTFLIGHT; k++) {
			if (!Ctrl->S[k].active) continue;	/* Not provided */
			n_errors += gmt_check_language (GMT, Ctrl->In.mode, Ctrl->S[k].file, k, NULL);
		}
		if (!n_errors && Ctrl->I.active) {	/* Must also check the include file, and open it for reading */
			n_errors += gmt_check_language (GMT, Ctrl->In.mode, Ctrl->I.file, 3, NULL);
			if (n_errors == 0 && ((Ctrl->I.fp = fopen (Ctrl->I.file, "r")) == NULL)) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open include script file %s\n", Ctrl->I.file);
				n_errors++;
			}
		}
		/* Open the main script for reading here */
		if (n_errors == 0 && ((Ctrl->In.fp = fopen (Ctrl->In.file, "r")) == NULL)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to open main script file %s\n", Ctrl->In.file);
			n_errors++;
		}
		if (n_errors == 0 && gmt_script_is_classic (GMT, Ctrl->In.fp)) {
			GMT_Report (API, GMT_MSG_ERROR, "Your main script file %s is not in GMT modern mode\n", Ctrl->In.file);
			n_errors++;
		}
		/* Make sure all BATCH_* variables are used with leading token ($, %) */
		if (n_errors == 0 && gmt_token_check (GMT, Ctrl->In.fp, "BATCH_", Ctrl->In.mode)) {
			n_errors++;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_batch (void *V_API, int mode, void *args) {
	int error = 0, precision;
	int (*run_script)(const char *);	/* pointer to system function or a dummy */

	unsigned int n_values = 0, n_jobs = 0, job, i_job, col, k, n_cores_unused, n_to_run;
	unsigned int n_jobs_not_started = 0, n_jobs_completed = 0, first_job = 0, data_job;

	bool done = false, n_written = false, has_text = false, is_classic = false, has_conf = false, issue_col0_par = false;

	static char *extension[3] = {"sh", "csh", "bat"}, *load[3] = {"source", "source", "call"}, var_token[4] = "$$%";
	static char *rmdir[3] = {"rm -rf", "rm -rf", "rd /s /q"}, *export[3] = {"export ", "setenv ", ""};
	static char *mvfile[3] = {"mv -f", "mv -f", "move /Y"};
	static char *createfile[3] = {"touch", "touch", "copy /b NUL"}, *rmfile[3] = {"rm -f", "rm -f", "del"};
	static char *cpconf[3] = {"\tcp -f %s .\n", "\tcp -f %s .\n", "\tcopy %s .\n"};
	static char *sys_cmd_nowait[3] = {"bash", "csh", "start /B"}, *sys_cmd_wait[3] = {"bash", "csh", "cmd /C"};

	char init_file[PATH_MAX] = {""}, state_tag[GMT_LEN16] = {""}, state_prefix[GMT_LEN64] = {""}, param_file[PATH_MAX] = {""};
	char pre_file[PATH_MAX] = {""}, post_file[PATH_MAX] = {""}, main_file[PATH_MAX] = {""}, line[PATH_MAX] = {""}, tmpwpath[PATH_MAX] = {""};
	char string[GMT_LEN128] = {""}, cmd[GMT_LEN256] = {""}, cleanup_file[PATH_MAX] = {""}, cwd[PATH_MAX] = {""}, conf_file[PATH_MAX];
	char completion_file[PATH_MAX] = {""}, topdir[PATH_MAX] = {""}, workdir[PATH_MAX] = {""}, datadir[PATH_MAX] = {""};

	double percent = 0.0;

	FILE *fp = NULL;

	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_OPTION *options = NULL;
	struct BATCH_STATUS *status = NULL;
	struct BATCH_CTRL *Ctrl = NULL;
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

	if (Ctrl->Q.scripts) {	/* No batch will be run, but scripts will be produced */
		GMT_Report (API, GMT_MSG_INFORMATION, "Dry-run enabled - Processing scripts will be created and any pre/post-flight scripts will be executed.\n");
		if (Ctrl->M.active) GMT_Report (API, GMT_MSG_INFORMATION, "A single script for job %d will be created and executed\n", Ctrl->M.job);
		run_script = gmt_dry_run_only;	/* This prevents the main job loop from executing the script */
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

	if (Ctrl->W.active) {	/* Do all work in a temp directory */
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

	if (!access ("gmt.conf", R_OK)) {	/* User has a gmt.conf file in the top directory that needs to be shared with the jobs */
		has_conf = true;
		sprintf (conf_file, "%s/gmt.conf", topdir);
		gmt_replace_backslash_in_path (conf_file);
		if (Ctrl->In.mode == GMT_DOS_MODE)	/* Make it suitable for DOS command line copying */
			gmt_strrepc (conf_file, '/', '\\');
	}

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

	gmt_replace_backslash_in_path (datadir);	/* Since we will be fprintf-ing the path we must use // for a slash */
	gmt_replace_backslash_in_path (workdir);

	/* Make a path to workdir that works on current architecture (for command line calls) */
	strncpy (tmpwpath, workdir, PATH_MAX);
	if (Ctrl->In.mode == GMT_DOS_MODE)	/* On Windows to do remove a file in a subdir one need to use back slashes */
		gmt_strrepc (tmpwpath, '/', '\\');

	/* Create the initialization file with settings common to all jobs */

	n_written = (n_jobs > 0);	/* Know the number of jobs already */
	sprintf (init_file, "batch_init.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_INFORMATION, "Create parameter initiation script %s\n", init_file);
	if ((fp = fopen (init_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s - exiting\n", init_file);
		batch_close_files (Ctrl);
		Return (GMT_ERROR_ON_FOPEN);
	}

	sprintf (string, "Static parameters set for processing sequence %s", Ctrl->N.prefix);
	gmt_set_comment (fp, Ctrl->In.mode, string);
	gmt_set_tvalue (fp, Ctrl->In.mode, true, "BATCH_PREFIX", Ctrl->N.prefix);
	if (n_written) gmt_set_ivalue (fp, Ctrl->In.mode, false, "BATCH_NJOBS", n_jobs);	/* Total jobs (write to init since known) */
	if (Ctrl->I.active) {	/* Append contents of an include file */
		gmt_set_comment (fp, Ctrl->In.mode, "Static parameters set via user include file");
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->I.fp)) {	/* Read the include file and copy to init script with some exceptions */
			if (gmt_is_gmtmodule (line, "begin")) continue;		/* Skip gmt begin */
			if (gmt_is_gmtmodule (line, "end")) continue;		/* Skip gmt end */
			if (strstr (line, "#!/")) continue;			/* Skip any leading shell incantation */
			if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
			fprintf (fp, "%s", line);				/* Just copy the line as is */
		}
		fclose (Ctrl->I.fp);	/* Done reading the include script */
	}
	fclose (fp);	/* Done writing the init script */

	if (Ctrl->S[BATCH_PREFLIGHT].active) {	/* Create the preflight script from the user's -Sf script */
		/* The preflight script must be modern mode */
		unsigned int rec = 0;
		sprintf (pre_file, "batch_preflight.%s", extension[Ctrl->In.mode]);
		is_classic = gmt_script_is_classic (GMT, Ctrl->S[BATCH_PREFLIGHT].fp);
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
		gmt_set_script (fp, Ctrl->In.mode);			/* Write 1st line of a script */
		gmt_set_comment (fp, Ctrl->In.mode, "Preflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);		/* Hardwire a Session Name since sub-shells may mess things up */
		if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately first */
			fprintf (fp, "set GMT_SESSION_NAME=1\n");
		else	/* On UNIX we may use the calling terminal or script's PID as the GMT_SESSION_NAME */
			gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[BATCH_PREFLIGHT].fp)) {	/* Read the preflight script and copy to the temporary preflight script with some exceptions */
			if (gmt_is_gmtmodule (line, "begin")) {	/* Need to insert the DIR_DATA statement */
				fprintf (fp, "%s", line);
				if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
				fprintf (fp, "\tgmt set DIR_DATA \"%s\"\n", datadir);
			}
			else if (!strstr (line, "#!/"))	 {	/* Skip any leading shell incantation since already placed by gmt_set_script */
				if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			}
			rec++;
		}
		fclose (Ctrl->S[BATCH_PREFLIGHT].fp);	/* Done reading the preflight script */
		fclose (fp);	/* Done writing the temporary preflight script */
#ifndef WIN32	/* Set executable bit if not Windows cmd */
		if (chmod (pre_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to make preflight script %s executable - exiting.\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
		/* Run the pre-flight now which may or may not create a <timefile> needed later via -T, as well as needed data files */
		sprintf (cmd, "%s %s", sys_cmd_wait[Ctrl->In.mode], pre_file);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running preflight script %s returned error %d - exiting.\n", pre_file, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Now we can complete the -T parsing since any given -T<timefile> has now been created by the preflight script */

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
			n_jobs = (unsigned int)D->n_records;	/* Number of records means number of jobs */
			n_values = (unsigned int)D->n_columns;	/* The number of per-job parameters we need to place into the per-job parameter files */
			has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		}
		else if (gmt_count_char (GMT, Ctrl->T.file, '/') == 2) {	/* Give a vector specification -Tmin/max/inc, call gmtmath to build the array */
			char output[GMT_VF_LEN] = {""}, cmd[GMT_LEN128] = {""};
			unsigned int V = GMT->current.setting.verbose;
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT|GMT_IS_REFERENCE, NULL, output) == GMT_NOTSET) {
				Return (API->error);
			}
			if (GMT->common.f.active[GMT_IN])
				sprintf (cmd, "-T%s -o1 -f%s --GMT_HISTORY=readonly T = %s", Ctrl->T.file, GMT->common.f.string, output);
			else
				sprintf (cmd, "-T%s -o1 --GMT_HISTORY=readonly T = %s", Ctrl->T.file, output);
			GMT_Report (API, GMT_MSG_INFORMATION, "Calling gmtmath with args %s\n", cmd);
			GMT->current.setting.verbose = GMT_MSG_ERROR;	/* So we don't get unwanted verbosity from gmtmath */
  			if (GMT_Call_Module (API, "gmtmath", GMT_MODULE_CMD, cmd)) {
				Return (API->error);	/* Some sort of failure */
			}
			GMT->current.setting.verbose = V;	/* Restore */
			if ((D = GMT_Read_VirtualFile (API, output)) == NULL) {	/* Load in the data array */
				Return (API->error);	/* Some sort of failure */
			}
			n_jobs = (unsigned int)D->n_records;	/* Number of records means number of jobs */
			n_values = (unsigned int)D->n_columns;	/* The number of per-job parameters we need to place into the per-job parameter files */
			has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
			if (strchr (Ctrl->T.file, 'T'))	/* Check here since gmtmath does not pass that info back */
				gmt_set_column_type (GMT, GMT_IN, GMT_X, GMT_IS_ABSTIME);	/* Set first input column type as absolute time */
		}
		else {	/* Just gave the number of jobs (we hope, or we got a bad filename and atoi should return 0) */
			n_jobs = atoi (Ctrl->T.file);
			issue_col0_par = true;
		}
	}
	if (n_jobs == 0) {	/* So not good... */
		GMT_Report (API, GMT_MSG_ERROR, "No jobs specified! - exiting.\n");
		fclose (Ctrl->In.fp);
		Return (GMT_RUNTIME_ERROR);
	}

	if (!n_written) {	/* Rewrite the init file to place the BATCH_NJOBS there */
		GMT_Report (API, GMT_MSG_INFORMATION, "Recreate parameter initiation script given njobs has been set %s\n", init_file);
		(void) gmt_remove_file (GMT, init_file);	/* Delete the first init file */
		if ((fp = fopen (init_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s - exiting\n", init_file);
			batch_close_files (Ctrl);
			Return (GMT_ERROR_ON_FOPEN);
		}

		sprintf (string, "Static parameters set for processing sequence %s", Ctrl->N.prefix);
		gmt_set_comment (fp, Ctrl->In.mode, string);
		gmt_set_tvalue (fp, Ctrl->In.mode, true, "BATCH_PREFIX", Ctrl->N.prefix);
		gmt_set_ivalue (fp, Ctrl->In.mode, false, "BATCH_NJOBS", n_jobs);	/* Total jobs (write to init since known) */
		if (Ctrl->I.active) {	/* Append contents of an include file */
			gmt_set_comment (fp, Ctrl->In.mode, "Static parameters set via user include file");
			while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->I.fp)) {	/* Read the include file and copy to init script with some exceptions */
				if (gmt_is_gmtmodule (line, "begin")) continue;		/* Skip gmt begin */
				if (gmt_is_gmtmodule (line, "end")) continue;		/* Skip gmt end */
				if (strstr (line, "#!/")) continue;			/* Skip any leading shell incantation */
				if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);				/* Just copy the line as is */
			}
			fclose (Ctrl->I.fp);	/* Done reading the include script */
		}
		fclose (fp);	/* Done writing the init script */
	}
	if (Ctrl->In.mode == GMT_DOS_MODE)
		gmt_strrepc (topdir, '/', '\\');	/* Temporarily make DOS compatible */
	n_to_run = (Ctrl->M.active) ? 1 : n_jobs;
	GMT_Report (API, GMT_MSG_INFORMATION, "Number of main processing jobs: %d\n", n_to_run);
	if (Ctrl->T.precision)	/* Precision was prescribed */
		precision = Ctrl->T.precision;
	else	/* Compute width from largest job number */
		precision = irint (ceil (log10 ((double)(Ctrl->T.start_job+n_jobs))+0.1));	/* Width needed to hold largest job number, guaranteed to give at least 1 */

	if (Ctrl->S[BATCH_POSTFLIGHT].active) {	/* Prepare the temporary postflight script */
		sprintf (post_file, "batch_postflight.%s", extension[Ctrl->In.mode]);
		GMT_Report (API, GMT_MSG_INFORMATION, "Create postflight script %s\n", post_file);
		if ((fp = fopen (post_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create postflight file %s - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		gmt_set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		gmt_set_comment (fp, Ctrl->In.mode, "Postflight script");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		fprintf (fp, "cd %s\n", topdir);		/* cd to the starting directory */
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a SESSION_NAME since sub-shells may mess things up */
		if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately */
			fprintf (fp, "set GMT_SESSION_NAME=1\n");
		else	/* On UNIX we may use the script's PID as GMT_SESSION_NAME */
			gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[BATCH_POSTFLIGHT].fp)) {	/* Read the postflight script and copy to the temporary postflight script with some exceptions */
			if (gmt_is_gmtmodule (line, "begin")) {
				fprintf (fp, "%s", line);	/* Allow args since the script may make a plot */
				if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
				fprintf (fp, "\tgmt set DIR_DATA \"%s\"\n", datadir);
			}
			else if (!strstr (line, "#!/"))	{	/* Skip any leading shell incantation since already placed */
				if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			}
		}
		fprintf (fp, "cd %s\n", tmpwpath);		/* cd back to the working directory */
		fclose (Ctrl->S[BATCH_POSTFLIGHT].fp);	/* Done reading the postflight script */
		fclose (fp);	/* Done writing the postflight script */
#ifndef WIN32	/* Set executable bit if not Windows cmd */
		if (chmod (post_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to make postflight script %s executable - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
	}

	/* Create parameter include files, one for each job */

	GMT_Report (API, GMT_MSG_INFORMATION, "Parameter files for main processing: %d\n", n_to_run);
	for (i_job = 0; i_job < n_jobs; i_job++) {
		job = data_job = i_job + Ctrl->T.start_job;	/* The job is normally same as data_job number */
		if (Ctrl->M.active && job != Ctrl->M.job) continue;	/* Just doing a single job for debugging */
		sprintf (state_tag, "%*.*d", precision, precision, job);
		sprintf (state_prefix, "batch_params_%s", state_tag);
		sprintf (param_file, "%s.%s", state_prefix, extension[Ctrl->In.mode]);
		if ((fp = fopen (param_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create job parameter file %s - exiting\n", param_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		sprintf (state_prefix, "Parameter file for job %s", state_tag);
		gmt_set_comment (fp, Ctrl->In.mode, state_prefix);
		sprintf (state_prefix, "%s_%s", Ctrl->N.prefix, state_tag);
		gmt_set_tvalue (fp, Ctrl->In.mode, false, "BATCH_NAME", state_prefix);	/* Current job name prefix (e.g., my_job_0003) */
		gmt_set_ivalue (fp, Ctrl->In.mode, false, "BATCH_JOB", data_job);		/* Current job number (e.g., 3) */
		gmt_set_tvalue (fp, Ctrl->In.mode, false, "BATCH_ITEM", state_tag);		/* Current job tag (formatted job number, e.g, 0003) */
		for (col = 0; col < n_values; col++) {	/* Derive job variables from this row in <timefile> and copy to each parameter file  as script variables */
			sprintf (string, "BATCH_COL%u", col);
			gmt_set_value (GMT, fp, Ctrl->In.mode, col, string, D->table[0]->segment[0]->data[col][data_job]);
		}
		if (issue_col0_par) gmt_set_ivalue (fp, Ctrl->In.mode, false, "BATCH_COL0", data_job);	/* Same as current job number */
		if (has_text) {	/* Also place any string parameter as a single string variable */
			gmt_set_tvalue (fp, Ctrl->In.mode, false, "BATCH_TEXT", D->table[0]->segment[0]->text[data_job]);
			if (Ctrl->T.split) {	/* Also split the string into individual words BATCH_WORD1, BATCH_WORD2, etc. */
				char *word = NULL, *trail = NULL, *orig = strdup (D->table[0]->segment[0]->text[data_job]);
				col = 0;
				trail = orig;
				while ((word = strsep (&trail, Ctrl->T.sep)) != NULL) {
					if (*word != '\0') {	/* Skip empty strings */
						sprintf (string, "BATCH_WORD%u", col++);
						gmt_set_tvalue (fp, Ctrl->In.mode, false, string, word);
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
	gmt_set_script (fp, Ctrl->In.mode);	/* Write 1st line of a script */
	gmt_set_comment (fp, Ctrl->In.mode, "Main job loop script");
	fprintf (fp, "%s", export[Ctrl->In.mode]);	/* Hardwire a GMT_SESSION_NAME since sub-shells may mess things up */
	if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to be the job number */
		fprintf (fp, "set GMT_SESSION_NAME=%c1\n", var_token[Ctrl->In.mode]);
	else	/* On UNIX we use the script's PID as GMT_SESSION_NAME */
		gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
	gmt_set_comment (fp, Ctrl->In.mode, "Include static and job-specific parameters");
	fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
	fprintf (fp, "%s batch_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the job parameters */
	fprintf (fp, "mkdir %s\n", gmt_place_var (Ctrl->In.mode, "BATCH_NAME"));	/* Make a temp directory for this job */
	fprintf (fp, "cd %s\n", gmt_place_var (Ctrl->In.mode, "BATCH_NAME"));		/* cd to the temp directory */
	while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the main script and copy to loop script, with some exceptions */
		if (gmt_is_gmtmodule (line, "begin")) {	/* Must insert DIR_DATA setting */
			fprintf (fp, "%s", line);
			if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
			fprintf (fp, "\tgmt set DIR_DATA \"%s\"\n", datadir);
		}
		else if (!strstr (line, "#!/")) {		/* Skip any leading shell incantation since already placed */
			if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
			fprintf (fp, "%s", line);	/* Just copy the line as is */
		}
	}
	fclose (Ctrl->In.fp);	/* Done reading the main script */
	/* Move job products up to main directory */
	fprintf (fp, "%s %s.* %s\n", mvfile[Ctrl->In.mode], gmt_place_var (Ctrl->In.mode, "BATCH_NAME"), topdir);
	fprintf (fp, "cd ..\n");	/* cd up to parent dir */
	/* Create completion file so batch knows this job is done */
	fprintf (fp, "%s %s.___\n", createfile[Ctrl->In.mode], gmt_place_var (Ctrl->In.mode, "BATCH_NAME"));
	if (!Ctrl->Q.active) {	/* Delete evidence; otherwise we want to leave debug evidence when doing a single job only */
		gmt_set_comment (fp, Ctrl->In.mode, "Remove job directory and job parameter file");
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], gmt_place_var (Ctrl->In.mode, "BATCH_NAME"));	/* Remove the work dir and any files in it */
		fprintf (fp, "%s batch_params_%c1.%s\n", rmfile[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Remove the parameter file for this job */
	}
	if (Ctrl->In.mode == GMT_DOS_MODE)	/* This is crucial to the "start /B ..." statement below to ensure the DOS process terminates */
		fprintf (fp, "exit\n");
	fclose (fp);	/* Done writing loop script */
	if (Ctrl->In.mode == GMT_DOS_MODE)
		gmt_strrepc (topdir, '\\', '/');	/* Revert */

#ifndef WIN32	/* Set executable bit if not Windows cmd */
	if (chmod (main_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to make script %s executable - exiting\n", main_file);
		Return (GMT_RUNTIME_ERROR);
	}
#endif

	/* Prepare the cleanup script */
	sprintf (cleanup_file, "batch_cleanup.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_INFORMATION, "Create cleanup script %s\n", cleanup_file);
	if ((fp = fopen (cleanup_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create cleanup file %s - exiting\n", cleanup_file);
		Return (GMT_ERROR_ON_FOPEN);
	}
	gmt_set_script (fp, Ctrl->In.mode);		/* Write 1st line of a script */
	if (Ctrl->W.active) {	/* Want to delete the entire work directory */
		gmt_set_comment (fp, Ctrl->In.mode, "Cleanup script removes working directory with job files");
		/* Delete the entire working directory with batch jobs and tmp files */
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], tmpwpath);
	}
	else {	/* Just delete the remaining script files */
		/* On Windows to do remove a file in a subdir one need to use back slashes */
		char dir_sep_ = (Ctrl->In.mode == GMT_DOS_MODE) ? '\\' : '/';
		GMT_Report (API, GMT_MSG_INFORMATION, "%u job product sets saved in directory: %s\n", n_jobs, workdir);
		if (Ctrl->S[BATCH_PREFLIGHT].active)	/* Remove the preflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep_, pre_file);
		if (Ctrl->S[BATCH_POSTFLIGHT].active)	/* Remove the postflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep_, post_file);
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep_, init_file);	/* Delete the init script */
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep_, main_file);	/* Delete the main script */
	}
	fclose (fp);
#ifndef WIN32	/* Set executable bit if not Windows cmd */
	if (chmod (cleanup_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to make cleanup script %s executable - exiting\n", cleanup_file);
		Return (GMT_RUNTIME_ERROR);
	}
#endif

	GMT_Report (API, GMT_MSG_INFORMATION, "Total jobs to process: %u\n", n_to_run);

	if (Ctrl->Q.scripts) {	/* No processing executed */
		Return (GMT_NOERROR);	/* We are done */
	}

	if (Ctrl->M.active) {	/* Just run that one job */
		sprintf (cmd, "%s %s %s", sys_cmd_wait[Ctrl->In.mode], main_file, state_tag);
		GMT_Report (API, GMT_MSG_INFORMATION, "Run master script %s %s\n", main_file, state_tag);
		if ((error = run_script (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running master script %s for argument %s returned error %d - exiting.\n", main_file, state_tag, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		Return (GMT_NOERROR);	/* We are done */
	}

	/* Finally, we can run all the jobs in a controlled loop, launching new parallel jobs as cores become available */

	i_job = first_job = 0; n_jobs_not_started = n_jobs;
	job = Ctrl->T.start_job;
	n_cores_unused = MAX (1, Ctrl->x.n_threads - 1);	/* Save one core for the main batch module thread */
	status = gmt_M_memory (GMT, NULL, n_jobs, struct BATCH_STATUS);	/* Used to keep track of job status */
	GMT_Report (API, GMT_MSG_INFORMATION, "Build jobs using %u cores\n", n_cores_unused);
	/* START PARALLEL EXECUTION OF JOB SCRIPTS */
	GMT_Report (API, GMT_MSG_INFORMATION, "Execute batch job scripts in parallel\n");
	while (!done) {	/* Keep running jobs until all jobs have completed */
		while (n_jobs_not_started && n_cores_unused) {	/* Launch new jobs if possible */
#ifdef WIN32
			if (Ctrl->In.mode < GMT_DOS_MODE)	/* A bash or csh run from Windows. Need to call via "start" to get parallel */
				sprintf (cmd, "start /B %s %s %*.*d", sys_cmd_nowait[Ctrl->In.mode], main_file, precision, precision, job);
			else						/* Running batch, so no need for the above trick */
				sprintf (cmd, "%s %s %*.*d &", sys_cmd_nowait[Ctrl->In.mode], main_file, precision, precision, job);
#else
			sprintf (cmd, "%s %s %*.*d &", sys_cmd_nowait[Ctrl->In.mode], main_file, precision, precision, job);
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
		gmt_sleep (BATCH_WAIT_TO_CHECK);	/* Wait 0.01 second - then check for the completion file */
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
			(void) gmt_remove_file (GMT, completion_file);	/* Delete the completion file */
			GMT_Report (API, GMT_MSG_INFORMATION, "Job %*.*d of %d completed [%5.1f %%]\n", precision, precision, k+1, n_jobs, percent);
		}
		/* Adjust first_job, if needed */
		while (first_job < n_jobs && status[first_job].completed) first_job++;
		if (n_jobs_completed == n_jobs) done = true;	/* All jobs completed! */
	}
	/* END PARALLEL EXECUTION OF JOB SCRIPTS */

	gmt_M_free (GMT, status);	/* Done with this structure array */

	if (Ctrl->S[BATCH_POSTFLIGHT].active) {
		/* Run post-flight now since all processing has completed */
		sprintf (cmd, "%s %s", sys_cmd_wait[Ctrl->In.mode], post_file);
		GMT_Report (API, GMT_MSG_INFORMATION, "Run postflight script %s\n", post_file);
		if ((error = run_script (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running postflight script %s returned error %d - exiting.\n", post_file, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (!Ctrl->Q.active) {
		/* Run cleanup script at the end */
		if (Ctrl->In.mode == GMT_DOS_MODE)
			error = system (cleanup_file);
		else {
			sprintf (cmd, "%s %s", sys_cmd_nowait[Ctrl->In.mode], cleanup_file);
			error = system (cmd);
		}
		if (error) {
			GMT_Report (API, GMT_MSG_ERROR, "Running cleanup script %s returned error %d - exiting.\n", cleanup_file, error);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Finally, delete the clean-up script separately since under DOS we got complaints when we had it delete itself (which works under *nix) */
	if (!Ctrl->Q.active && gmt_remove_file (GMT, cleanup_file)) {	/* Delete the cleanup script itself */
		GMT_Report (API, GMT_MSG_ERROR, "Unable to delete the cleanup script %s.\n", cleanup_file);
		Return (GMT_RUNTIME_ERROR);
	}

	/* Cd back up to the parent directory */
	if (chdir (topdir)) {	/* Should never happen but we should check */
		GMT_Report (API, GMT_MSG_ERROR, "Unable to change directory to starting directory - exiting.\n");
		perror (topdir);
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->Z.active) {	/* Delete input scripts */
		if ((error = batch_delete_scripts (GMT, Ctrl)))
			Return (error);
	}

	Return (GMT_NOERROR);
}
