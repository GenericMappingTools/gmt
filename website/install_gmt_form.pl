#!/usr/bin/perl
#
#       $Id: install_gmt_form.pl,v 1.10 2004-08-18 23:19:45 pwessel Exp $
#
#	Parses the input provided by the install form
#	(Now in Bourne shell format)
#
#	Updated to work with 4.0 input

$webmaster = "gmt\@soest\.hawaii\.edu";

# Create return plain text file

print "Content-type: text/plain", "\n";
print "Status: 200 OK", "\n\n";

&parse_form_data (*gmt_form);

# Assign internal variables for each form item:

$gmt_version	= $gmt_form{'gmt_version'};
$form_version	= $gmt_form{'form_version'};
$zip		= $gmt_form{'radio_format'};
$unit		= $gmt_form{'radio_unit'};
$eps		= $gmt_form{'radio_eps'};
$flock		= $gmt_form{'radio_flock'};
$cdf		= $gmt_form{'radio_netcdf'};
$ftpmode	= $gmt_form{'radio_ftpmode'};
$cdf_path	= $gmt_form{'netcdf_dir'};
$site		= $gmt_form{'radio_site'};
$get_progs	= $gmt_form{'checkbox_progs'};
$get_libs	= $gmt_form{'checkbox_libs'};
$get_ps		= $gmt_form{'checkbox_ps'};
$get_pdf	= $gmt_form{'checkbox_pdf'};
$get_man	= $gmt_form{'checkbox_man'};
$get_tut	= $gmt_form{'checkbox_tut'};
$get_web	= $gmt_form{'checkbox_web'};
$get_scripts	= $gmt_form{'checkbox_scripts'};
$get_suppl	= $gmt_form{'checkbox_suppl'};
$get_high	= $gmt_form{'checkbox_high'};
$get_full	= $gmt_form{'checkbox_full'};
$get_triangle	= $gmt_form{'checkbox_triangle'};
$libtype	= $gmt_form{'radio_link'};
$cc		= $gmt_form{'cc'};
$custom_cc	= $gmt_form{'custom_cc'};
$make		= $gmt_form{'make'};
$custom_make	= $gmt_form{'custom_make'};
$gmt_bin	= $gmt_form{'gmt_bin'};
$gmt_lib	= $gmt_form{'gmt_lib'};
$gmt_include	= $gmt_form{'gmt_include'};
$gmt_share	= $gmt_form{'gmt_share'};
$gmt_man	= $gmt_form{'gmt_man'};
$gmt_mansect	= $gmt_form{'gmt_mansect'};
$gmt_web	= $gmt_form{'gmt_web'};
$gmt_def	= $gmt_form{'gmt_def'};
$gmt_coast	= $gmt_form{'radio_coast'};
$gmt_cli_dir	= $gmt_form{'gmt_cli_dir'};
$gmt_h_dir	= $gmt_form{'gmt_h_dir'};
$gmt_f_dir	= $gmt_form{'gmt_f_dir'};
$get_dbase	= $gmt_form{'checkbox_dbase'};
$get_gshhs	= $gmt_form{'checkbox_gshhs'};
$get_imgsrc	= $gmt_form{'checkbox_imgsrc'};
$get_meca	= $gmt_form{'checkbox_meca'};
$get_mgd77	= $gmt_form{'checkbox_mgd77'};
$get_mgg	= $gmt_form{'checkbox_mgg'};
$get_mex	= $gmt_form{'checkbox_mex'};
$get_misc	= $gmt_form{'checkbox_misc'};
$get_segyprogs	= $gmt_form{'checkbox_segyprogs'};
$get_spotter	= $gmt_form{'checkbox_spotter'};
$get_x2sys	= $gmt_form{'checkbox_x2sys'};
$get_x_system	= $gmt_form{'checkbox_x_system'};
$get_xgrid	= $gmt_form{'checkbox_xgrid'};
$matlab_dir	= $gmt_form{'matlab_dir'};
$delete		= $gmt_form{'checkbox_delete'};
$run		= $gmt_form{'checkbox_run'};


$now		= `date`;
chop($now);

# Start printing the parameter file to be returned

print <<EOF;
# This file contains parameters needed by the install script
# for GMT Version $gmt_version or later.  Give this parameter file
# as the argument to the install_gmt script and the whole
# installation process can be placed in the background.
# Default answers will be selected where none is given.
# You can edit the values, but do not remove definitions!
#
# Assembled by gmt_install_form.html, $form_version
# Processed by install_gmt_form.pl $Revision: 1.10 $,  on
#
#	$now
#
# Do NOT add any spaces around the = signs.  The
# file MUST conform to Bourne shell syntax
#---------------------------------------------
#	SYSTEM UTILITIES
#---------------------------------------------
EOF

if ($zip eq "default") {
	print "GMT_expand=\n";
}
else {
	print "GMT_expand=", $zip, "\n";
}
@k = split (/\s+/, $make);
if ($k[0] eq "1.") {
	print "GMT_make=make\n";
}
else {
	print "GMT_make=", $custom_make, "\n";
}

print "#---------------------------------------------\n";
print "#	NETCDF SECTION\n";
print "#---------------------------------------------\n";
if ($cdf eq "get") {
	print "netcdf_ftp=y\n";
	print "netcdf_install=y\n";
}
elsif ($cdf eq "install") {
	print "netcdf_ftp=n\n";
	print "netcdf_install=y\n";
}
else {
	print "netcdf_ftp=n\n";
	print "netcdf_install=n\n";
}
print "netcdf_path=", $cdf_path, "\n";
print "passive_ftp=";
if ($ftpmode eq "passive") {
	print "y\n";
}
else {
	print "n\n";
}
print "#---------------------------------------------\n";
print "#	GMT FTP SECTION\n";
print "#---------------------------------------------\n";
if ($site eq "7") {
	print "GMT_ftp=n\n";
}
else {
	print "GMT_ftp=y\n";
}
print "GMT_ftpsite=", $site, "\n";
print "GMT_get_progs=";
if ($get_progs eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_share=";
if ($get_libs eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_high=";
if ($get_high eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_full=";
if ($get_full eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_suppl=";
if ($get_suppl eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_scripts=";
if ($get_scripts eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_ps=";
if ($get_ps eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_pdf=";
if ($get_pdf eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_man=";
if ($get_man eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_tut=";
if ($get_tut eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_web=";
if ($get_web eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_get_triangle=";
if ($get_triangle eq "on") {
	print "y\n";
}
else {
	print "n\n";
}

print "#---------------------------------------------\n";
print "#	GMT SUPPLEMENTS SELECT SECTION\n";
print "#---------------------------------------------\n";

print "GMT_suppl_dbase=";
if ($get_dbase eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_gshhs=";
if ($get_gshhs eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_imgsrc=";
if ($get_imgsrc eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_meca=";
if ($get_meca eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_mex=";
if ($get_mex eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_mgd77=";
if ($get_mgd77 eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_mgg=";
if ($get_mgg eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_misc=";
if ($get_misc eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_segyprogs=";
if ($get_segyprogs eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_spotter=";
if ($get_spotter eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_x2sys=";
if ($get_x2sys eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_x_system=";
if ($get_x_system eq "on") {
	print "y\n";
}
else {
	print "n\n";
}
print "GMT_suppl_xgrid=";
if ($get_xgrid eq "on") {
	print "y\n";
}
else {
	print "n\n";
}

print "#---------------------------------------------\n";
print "#	GMT ENVIRONMENT SECTION\n";
print "#---------------------------------------------\n";

if ($unit eq "SI") {
	print "GMT_si=y\n";
}
else {
	print "GMT_si=n\n";
}

if ($eps eq "PS") {
	print "GMT_ps=y\n";
}
else {
	print "GMT_ps=n\n";
}

if ($gmt_def eq "" && $gmt_share ne "") {
	$gmt_def =$gmt_share;
	$gmt_def =~ s#/share$##;	# Remove trailing /share
}
print "GMT_def=", $gmt_def, "\n";
print "GMT_share=", $gmt_share, "\n";
print "GMT_bin=", $gmt_bin, "\n";
print "GMT_lib=", $gmt_lib, "\n";
print "GMT_include=", $gmt_include, "\n";
print "GMT_man=", $gmt_man, "\n";
print "GMT_web=", $gmt_web, "\n";

if ($gmt_coast eq "all") {
	print "GMT_dir_full=\nGMT_dir_high=\nGMT_dir_cli=\n";
}
else {
	print "GMT_dir_full=", $gmt_full_dir, "\n";
	print "GMT_dir_high=", $gmt_high_dir, "\n";
	print "GMT_dir_cli=", $gmt_cli_dir, "\n";
}

print "GMT_mansect=", $gmt_mansect, "\n";

print "#---------------------------------------------\n";
print "#	COMPILING & LINKING SECTION\n";
print "#---------------------------------------------\n";

if ($libtype eq "Static") {
	print "GMT_sharedlib=n\n";
}
else {
	print "GMT_sharedlib=y\n";
}

@k = split (/\s+/, $cc);
if ($k[0] eq "1.") {
	print "GMT_cc=cc\n";
}
elsif ($k[0] eq "2.") {
	print "GMT_cc=gcc\n";
}
else {
	print "GMT_cc=", $custom_cc, "\n";
}

if ($flock eq "Lock") {
	print "GMT_flock=y\n";
}
else {
	print "GMT_flock=n\n";
}

if ($get_triangle eq "on") {
	print "GMT_triangle=y\n";
}
else {
	print "GMT_triangle=n\n";
}

print "#---------------------------------------------\n";
print "#	TEST & PRINT SECTION\n";
print "#---------------------------------------------\n";

if ($run eq "on") {
	print "GMT_run_examples=y\n";
}
else {
	print "GMT_run_examples=n\n";
}
if ($delete eq "on") {
	print "GMT_delete=y\n";
}
else {
	print "GMT_delete=n\n";
}
if ($matlab_dir ne "/usr/local/matlab" && $matlab_dir ne "") {
	print "MATDIR=", $matlab_dir, "\n";
}


exit(0);

sub parse_form_data
{
	local (*FORM_DATA) = @_;

	local ( $request_method, $query_string, @key_value_pairs, $key_value, $key, $value);

	$request_method = $ENV{'REQUEST_METHOD'};

	if ($request_method eq "GET") {
		$query_string = $ENV{'QUERY_STRING'};
	} elsif ($request_method eq "POST") {
		read (STDIN, $query_string, $ENV{'CONTENT_LENGTH'});
	} else {
		&return_error (500, "Server Error", "Server uses unsupported method");
	}

	@key_value_pairs = split (/&/, $query_string);

	foreach $key_value (@key_value_pairs) {

		($key, $value) = split (/=/, $key_value);
		$value =~ tr/+/ /;
		$value =~ s/%([\dA-Fa-f][\dA-Fa-f])/pack ("C", hex ($1))/eg;

		if (defined ($FORM_DATA{$key})) {
			$FORM_DATA{$key} = join ("+", $FORM_DATA{$key}, $value);
		} else {
			$FORM_DATA{$key} = $value;
		}
	}
}

sub return_error
{
	local ($status, $keyword, $message) = @_;

	print "Content-type: text/html", "\n";
	print "Status: ", $status, " ", $keyword, "\n\n";
	
	print <<End_of_Error;

<title>CGI Program - Unexpected Error</title>
<h1>$keyword</h1>
<hr>$message<hr>
Please contact $webmaster for more information.

End_of_Error

	exit (1);
}
