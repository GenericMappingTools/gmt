#!/usr/bin/perl

#       $Id: gmt_form.pl,v 1.6 2002-09-29 22:27:24 pwessel Exp $

$webmaster = "gmt-team\@hawaii\.edu";

# Create return HTML Test file

print "Content-type: text/html", "\n";
print "Status: 200 OK", "\n\n";
&parse_form_data (*gmt_form);

$lon   = $gmt_form{'lon'};
$lat   = $gmt_form{'lat'};
$g_lon = &get_degrees ($lon);
$g_lat = &get_degrees ($lat);
$caste = $gmt_form{'radio_status'};
$group = $gmt_form{'checkbox_group'};
$help  = $gmt_form{'checkbox_help'};

$First_Name  = $gmt_form{'first_name'};
$Last_Name   = $gmt_form{'last_name'};
$Institution = $gmt_form{'institution'};
$City        = $gmt_form{'city'};
$State       = $gmt_form{'state'};
$Country     = $gmt_form{'country'};
$Field       = $gmt_form{'field'};
$users       = $gmt_form{'n_users'};
$Machine     = $gmt_form{'machine'};
$OS          = $gmt_form{'os'};
$Comments    = $gmt_form{'comments'};
$Email       = $gmt_form{'email'};

if ($caste ne "guru" && $caste ne "user") {
	$caste = "admin";
}

# Send mail to the gmt registration service

open (TMPFILE, ">>/tmp/gmtregistration") || die "Cannot write to tmp/gmtregistration";
print TMPFILE <<EOF;

First_Name  : $First_Name
Last_Name   : $Last_Name
Institution : $Institution
City        : $City
State       : $State
Country     : $Country
Longitude   : $g_lon
Latitude    : $g_lat
# of users  : $users
Field       : $Field
Machine(s)  : $Machine
OS(s)       : $OS
E-mail      : $Email
Class       : $caste
EOF
if ($group eq "yes") {
	print TMPFILE "Subscribe gmtgroup\n";
}
if ($help eq "yes") {
	print TMPFILE "Subscribe gmthelp\n";
}
print TMPFILE "#-- Comments --\n";
print TMPFILE $Comments, "\n";

close (TMPFILE);

print <<EOF;
<HTML>
<HEAD>
</HEAD>
<BODY>
<h2><center>GMT Registration successfully received</center></h2>
Your GMT registration has been processed and accepted.  The usage map is
updated daily so please check back to see if your location was added to
the map.  Also, you will be added to the mailinglist(s) within 1 day (if
you choose to sign up for one of them).  Thank you for registrating with us.
The following is the information we received:
<HR>
<h3>
First_Name  : $First_Name<br>
Last_Name   : $Last_Name<br>
Institution : $Institution<br>
City        : $City<br>
State       : $State<br>
Country     : $Country<br>
Longitude   : $g_lon<br>
Latitude    : $g_lat<br>
Status      : $caste<br>
# of users  : $users<br>
Field       : $Field<br>
Machine(s)  : $Machine<br>
OS(s)       : $OS<br>
Lon         : $g_lon<br>
Lat         : $g_lat<br>
EOF
if ($group eq "yes") {
	print "Subscribe : gmtgroup<br>\n";
}
if ($help eq "yes") {
	print "Subscribe : gmthelp<br>\n";
}
if ($Comments) {
	print "Comments :<br>\n";
	print $Comments, "<br>\n";
}
print <<EOF;
</h3>
<HR>
<A HREF="http://gmt.soest.hawaii.edu">
<IMG SRC="/gmt/gmt/images/gmt_small_logo.gif" ALT="RETURN">
Return to GMT home page.
</A>
</BODY>
</HTML>
EOF

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

sub get_degrees
{

	local ($lon) = @_;

#	 Process the longitude string

	$lon =~ tr/a-z/A-Z/;
	$lon =~ s/ //;		# Take out spaces
	$i = index ($lon, "W");
	$j = index ($lon, "S");
	if ($i > 0 || $j > 0) {
		$sign_lon = -1.0;
	} else {
		$sign_lon = 1.0;
	}
	@fields = split (/:/, $lon);
	$factor = 1.0;
	$geodetic_lon = 0.0;
	foreach $x (@fields) {
		$geodetic_lon += ($x * $factor);
		$factor /= 60.0;
	}
	$geodetic_lon *= $sign_lon;
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
