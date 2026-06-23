#!/usr/bin/env bash
# Regression test for GMT_DATA_SERVER alias redirect resolution.
#
# Background: Alias server names (e.g., china, australia) expand to Hover DNS
# URL-forwards that redirect to the real mirror but strip the file path.
# Before the fix, GMT constructed download URLs using the alias base (e.g.,
# http://china.generic-mapping-tools.org/server/earth/earth_relief/file.grd),
# which Hover would redirect to the mirror root — discarding the file path and
# returning an HTML directory listing instead of the requested file.
#
# The fix (gmtremote_resolve_redirect in gmt_remote.c) follows redirects once
# at session start to discover the real mirror base URL, then constructs all
# file paths against that resolved URL.
#
# With GMT_DATA_SERVER=static (CI default) this test uses a locally cached
# grid and verifies the grdinfo output is sane. With a live alias server it
# also exercises the redirect resolver; a broken resolver would return an HTML
# body that GMT cannot parse as a netCDF grid, causing the test to fail.

# Remove any cached copy to force a real download from the server
rm -f "${HOME}/.gmt/server/earth/earth_relief/earth_relief_01d_p.grd"

# Force an alias server name so the alias expansion and redirect resolver code
# in gmt_dataserver_url() is always exercised, regardless of the build-time
# GMT_DATA_SERVER default.  oceania is a stable direct server (no DNS forward).
export GMT_DATA_SERVER=oceania

# Download (or use cached) global 1 arc-degree relief grid and get its range
gmt grdinfo @earth_relief_01d_p.grd -I- > got.txt 2>err.txt

# A valid global grid must report exactly this region
echo "-R-180/180/-90/90" > expected.txt

diff expected.txt got.txt > fail
