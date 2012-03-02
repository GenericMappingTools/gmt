#!/bin/sh
dir=$1
GMT_VERSION=`grep GMT_VERSION ../src/config.mk | awk '{print $3}'`
GSHHS_VERSION=`grep GSHHS_VERSION ../src/config.mk | awk '{print $3}'`

while read ftp tag name; do
	sed -e s/GMT_VERSION/${GMT_VERSION}/g -e s/GSHHS_VERSION/${GSHHS_VERSION}/g gmt_windows_template.html > gmt_windows_$tag.html
	cat <<- EOF >> gmt_windows_$tag.html
	<B>Selected ftp site: $name</B><P>
	<table border=0 cellspacing=0 cellpadding=4 bgcolor="#EEE8AA" bordercolor="black">
	  <tr>
	    <td><A HREF="$ftp/$dir/windows/gmt-${GMT_VERSION}_install32.exe">gmt-${GMT_VERSION}_install32.exe</A></td>
	    <td>The 32-bit GMT distribution (programs, libraries, supplements, complete set of GSHHS coastlines, HTML docs, examples).</td>
	  </tr>
	  <tr>
	    <td><A HREF="$ftp/$dir/windows/gmt-${GMT_VERSION}_install64.exe">gmt-${GMT_VERSION}_install64.exe</A></td>
	    <td>The 64-bit GMT distribution (programs, libraries, supplements, complete set of GSHHS coastlines, HTML docs, examples).</td>
	  </tr>
	  <tr>
	    <td><A HREF="$ftp/$dir/windows/gmt-${GMT_VERSION}_pdf_install.exe">gmt-${GMT_VERSION}_pdf_install.exe</A></td>
	    <td>The complete GMT PDF documentation.</td>
	  </tr>
	</table>
	<HR>
	</BODY>
	</HTML>
	EOF
done < mirrors.txt
