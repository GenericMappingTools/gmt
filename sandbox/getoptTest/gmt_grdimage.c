#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "gmt_cmdline_grdimage.h"

static struct gengetopt_args_info args_info;

void gmt_print_help (void) {
	int i = 0;
	fprintf(stderr, "\n%s\n\n", gengetopt_args_info_usage);
	while (gengetopt_args_info_help[i])
		fprintf(stderr, "%s\n", gengetopt_args_info_help[i++]);
}

void gmt_print_full_help (void) {
	int i = 0;
	fprintf(stderr, "\n%s\n\n", gengetopt_args_info_usage);
	while (gengetopt_args_info_full_help[i])
		fprintf(stderr, "%s\n", gengetopt_args_info_full_help[i++]);
}

void gmt_print_detailed_help (void) {
	int i = 0;
	fprintf(stderr, "\n%s\n\n", gengetopt_args_info_usage);
	while (gengetopt_args_info_detailed_help[i])
		fprintf(stderr, "%s\n", gengetopt_args_info_detailed_help[i++]);
}

void gmt_handle_usage (void) {
	if (args_info.version_given)
		printf ("%s\n", GMT_CMDLINE_PARSER_VERSION);
	else if (args_info.detailed_help_given)
		//gmt_print_detailed_help();
		gmt_cmdline_parser_print_detailed_help();
	else if (args_info.full_help_given)
		//gmt_print_full_help();
		gmt_cmdline_parser_print_full_help();
	else {
		//gmt_print_help();
		gmt_cmdline_parser_print_help();
		fprintf (stderr, "\nSpecify --full-help or --detailed-help to see the complete list of options.\n");
	}
}

int main (int argc, char **argv) {
	int status = 0;
	static struct gmt_cmdline_parser_params params;

	/* init cmdline parser params */
	gmt_cmdline_parser_params_init(&params);
	params.check_ambiguity = 1;
	//params.override = 1;

	/* call cmdline parser */
	status = gmt_cmdline_parser_ext (argc, argv, &args_info, &params);

	/* print version */
	if (args_info.version_given) {
		gmt_handle_usage();
		return 0;
	}

	/* check unamed options number */
	if (status == 0 && !(args_info.inputs_num == 1 || args_info.inputs_num == 3)) {
		fprintf (stderr, "%s: Need to specify grd_z | grd_r grd_g grd_b\n", argv[0]);
		status = 2;
	}

	if (args_info.help_given
			|| args_info.full_help_given
			|| args_info.detailed_help_given
			|| args_info.version_given) {
		status = 3;
	}

	if ( status !=0 ) {
		gmt_handle_usage();
		return status;
	}

	/* dump to stdout */
	printf ("\nContents of the option struct:\n");
	gmt_cmdline_parser_dump (stdout, &args_info);

	/* dump to file (for parsing in the next gmt command) */
	//gmt_cmdline_parser_file_save ("gmtcommands.txt", &args_info);

	gmt_cmdline_parser_free (&args_info);
	return status;
}
