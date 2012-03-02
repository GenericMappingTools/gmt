<!--    $Id$      -->
<!-- This simple PHP script will display the latest ChangeLog written to the gmt web directory      -->
<!-- from the subversion server, which happens once each night      -->
<HTML>
<HEAD>
<TITLE>GMT ChangeLog</TITLE>
</HEAD>
<BODY>
<H2><CENTER>Nightly Snapshot of the GMT Subversion ChangeLog</CENTER></H2>
<HR>
<?php
print "<pre>";
system( "sed -e 's/</\&lt;/g' -e 's/>/\&gt;/g' ChangeLog", $return );
print "</pre>";
?>
</BODY>
</HTML>
