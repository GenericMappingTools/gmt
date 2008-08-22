<!--    $Id: gmt_changelog.php,v 1.3 2008-08-22 12:48:41 remko Exp $      -->
<!-- This simple PHP script will display the latest ChangeLog written to the gmt web directory      -->
<!-- from the cvs server, which happens once each night      -->
<HTML>
<HEAD>
<TITLE>GMT ChangeLog</TITLE>
</HEAD>
<BODY>
<H2><CENTER>Nightly Snapshot of the GMT CVS ChangeLog</CENTER></H2>
<HR>
<?php
print "<pre>";
system( "cat ChangeLog", $return );
print "</pre>";
?>
</BODY>
</HTML>
