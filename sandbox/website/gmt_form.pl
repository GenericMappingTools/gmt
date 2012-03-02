#!/usr/bin/perl

#       $Id$

$webmaster = "gmt-team\@hawaii\.edu";

# Create return HTML Test file

print "Content-type: text/html", "\n";
print "Status: 200 OK", "\n\n";
&parse_form_data (*gmt_form);

$lon_deg   = $gmt_form{'lon_deg'};
$lon_min   = $gmt_form{'lon_min'};
$lon_sec   = $gmt_form{'lon_sec'};
$lon_dir   = $gmt_form{'lon_dir'};
$lat_deg   = $gmt_form{'lat_deg'};
$lat_min   = $gmt_form{'lat_min'};
$lat_sec   = $gmt_form{'lat_sec'};
$lat_dir   = $gmt_form{'lat_dir'};

$lon_g = $lon_deg + $lon_min / 60.0 + $lon_sec / 3600.0;
if ($lon_dir eq "W") {
	$lon_g = -$lon_g;
}
$lat_g = $lat_deg + $lat_min / 60.0 + $lat_sec / 3600.0;
if ($lat_dir eq "S") {
	$lat_g = -$lat_g;
}

# Write gmt registration file

open (TMPFILE, ">>/tmp/gmtregistration") || die "Cannot write to tmp/gmtregistration";
printf TMPFILE "%g\t%g\n", $lon_g, $lat_g;
close (TMPFILE);
print <<EOF;
<HTML>
<HEAD>
</HEAD>
<BODY>
<h2><center>Coordinates successfully received</center></h2>
We have recorded your site coordinates as<P>
EOF
printf "<B>Longitude: %g  Latitude: %g</B><P>\n", $lon_g, $lat_g;
print <<EOF;
The usage map is updated daily so please check back to see if your location was added to
the map after we more carefully process the data.  Thank you for participating.
<HR>
<A HREF="http://gmt.soest.hawaii.edu">
<IMG SRC="/gmt/gmt/gmt_small_logo.gif" ALT="RETURN">
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
