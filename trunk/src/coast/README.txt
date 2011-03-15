#	$Id: README.txt,v 1.6 2011-03-15 02:06:37 guru Exp $

README for coast supplement:

Updated:	Sept 23, 2005
Author:		Paul Wessel

The following is based on Paul Wessel's memory and by peeking into
all the source files.  I believe the following to be true:

1. We used wvs_read.c to pull segments of the CD-ROM.  Segments
   were stored in a binary integer format called "segment" format.
   Most of the segment_*.c programs deal with these segments.
   Basically, most coastlines where chopped into smaller pieces
   and these have to be spliced back together.  This was made
   harder by pieces not fitting quite back, thus necessetating
   much processing.
2. Once segments were clean and spliced together, we wrote out
   a binary database of polygons.  Each polygon had a header and
   most polygon_*.c programs read/write this format.  Some also
   read segment format and write polygon format.
3. Once polygons were made we needed to determine the hierarchical
   levels of each polygon (1 = shoreline, 2 is lake, 3 is island-
   in-lake, and so on (4 was the highest in the data).
4. We found that many polygons crossed other polygons.  Most of this
   was caused by having to merge two different data sets: (A) The
   WVS shoreline data set only had ocean/land shorelines, but at high
   resolution. (B) the older WDBII data had lakes and higher but also
   had shorelines, but at lower resolution.  MUCH time was spent
   trying to sort out which features where duplicated and which were
   cased by different kinks near river deltas etc.
5. When crossovers were found we had to manually edit points so that
   the crossings were avoided.  Very often this were between WVS and
   WDB polygons but sometimes between 2 WVS polygons.  Adding some
   intermediate points would most of the time fix the problem.
6. Once all the polygons were clean and free of crossovers, we had
   the final full resolution polygon files.  This is essentially
   the full GSHHS database.
7. We would now decimate the data down to high, intermediate, low,
   and crude resolution using the D-P algorithm.  This would introduce
   new crossovers in the derived products which again had to be
   manually corrected.  This gave the lower resolution GSHHS files.
8. We now had to bin the polygons.  This is done by first running
   polygon_to_bins.c which splits polygons into segments again
   (sounds like were we started but now we know how to put them
   back together and their levels.  They are also organized so that
   when traversed along the path your left hand points to land).
   The binned files are then merged and reformatted to the final
   GMT netCDF format that pscoast etc can understand.
   
Here is a list of the programs and a sort description of what I
think they do.  There were originally many more but most were
either abandoned or were simple *_fix.c programs that corrected
a mistake introduced by a step before we could fix the program.
Many programs were not used at all since the original 1995? stage
but some had to be recompiled again in 2004 when I updated some
segments.  Compiler flags etc can change.  Watch for 32/64 bit
issues on 64-bit platforms.

line_shrink.c:
	Simplifies a line segment using the D-P function.
linemaker.c
	Creates the netCDF border and river files from segment files
lines_to_bins.c:
	Used to read the political boundaries and river files and
	bin them, similar to polygons_to_bins.
poly_check_subs.c:
	Various functions used to check polygons.
polygon_bincount.c:
	polygon_bincount calculates # points pr bin.  Probably used to
	figure out how many points would land in each bin and what the
	maximum number of points in a bin would be.
polygon_checkarea.c
	This program compares all polygons to determine if there are
	duplicates.  Decision is based on size of area and location of
	the centroid and number of points.  If two sets match then we
	most likely have a duplicate polygon.
polygon_consistency.c:
	polygon_consistency checks for proper closure and crossings
	within polygons
polygon_deldups.c:
	OLD, not used lately.
polygon_dump.c:
	Makes ASCII dump of entire database or individual polygon.#
	ASCII files.
polygon_extract.c:
	Similar to polygon_dump; allows for a subset of numbered polygons
	to be extracted.
polygon_extract2.c:
	OLD, seems to have restriction on w/e/s/n for extraction?
polygon_extract_all.c:
	OLD, seems to have restriction on w/e/s/n for extraction?
polygon_final_info.c:
	Generates the one-line-per-polygon information table
polygon_findarea.c:
	Computes the area of all polygons
polygon_findlevel.c:
	Determines which hierarchical level each polygons belong to.
polygon_fix.c:
	Appears to reassign polygon iDs sequentially fpr a binary file
	with some (not all) polygons.  Also sets the level to a constant
	for all those polygons, hence they should all be lakes, islands,
	etc.
polygon_fixlevel.c:
	OLD, will reset the level of all polygons to the given level, and
	if the old levels differed it might reverse the polygon.
polygon_get.c:
	Seems to extract polygons inside w/e/s/n that matches the given level
polygon_id.c:
	Finds ID of polygon closest to given lon,lat point
polygon_merge.c:
	Merges polygons from two files.  If a polygon appears on both files
	then the one in the second file is used.
polygon_report.c:
	Counts the number of polygons for each level
polygon_restore.c:
	Creates a database by converting individual ASCII polygon files. 
polygon_set.c:
	Sets the specified level of a specified polygon ID; if level differs
	we might have to reverse the path.
polygon_setnodes.c:
	Determines the levels of grid nodes given the polygon levels.
polygon_setwesn.c:
	Resets the w/e/s/n values for each polygon by scanning the coordinates.
polygon_shrink.c
	Apply D-P algorithm to create a decimated data base
polygon_sort.c
	Read two data base and write out one sorted by polygon point length.
polygon_stats.c:
	Report statistics of the polygon point separations.
polygon_to_bins.c
	Chop polygons into segments defined by a grid spacing.
polygon_to_gshhs.c
	COnvert polygons to the GSHHS database format
polygon_update.c:
	Update data base by (1) deleting listed polygons and (2) updating others
	from pol/polygon.# ascii tabes.
polygon_xover.c:
	Determine crossovers between all pairs of polygons.  The causes must be
	examined manually and resolved so that there are no crossovers when you
	are finished.
read_wvs.c:
	Modified version of program distributed with the NGDC CD-ROM with the WVS
	data set that reads the segment files on the CD.
segment_clean.c:
	Cleans up segments, verifies coordinates ranges, checks for duplicates, spikes
	and crossing lines along segments.
segment_connect.c:
	Connects segments from a polygon into closed polygons.  We do this by finding
	segments whose endpoints are the closest.
segment_dump.c:
	Converts segment database to multisegment ascii table.
segment_final_dump.c:
	Converts polygons in segment binary form to polygon binary form.
segment_restore.c:
	Creates a segment database by converting individual ASCII segment files. 
segment_report.c:
	Reports statistics on how many segment of each level
shoremaker.c:
	Takes output of polygons_to_bin and produce the final netCDF coastline files.
wvs_crosscheck.c:
	Determines crossovers and automatically chops off the offending points, then
	recalculates crossovers.  If still crossings then give errors.
wvs_segment_dump.c:
	Dump individual segments to prefix.# ascii files.
wvs_segment_restore2.c:
	Converts a list of ascii segment files to a binary segment file

Some function libraries:

douglas_peucker.c:
	D-P algorithm for line simplification as a function.
coast_io.c:
	Set of subroutines used by other programs
string_check_subs.c:
	Subroutines for checking segments.
poly_misc_subs.c:
	Subroutines for misc polygon operations.

The makefile will create a libcoast.a out of those four files.
The makefile will do alot of the steps.  One should study the makefile
and see what the options are.  For instance, to bin all the polygons
after the cleaning is completed one can do "make bin".
While most of this code might compile under Windows there is no guarantee
it will work well there and it is strongly recommended to do all these
steps under a Unix-like OS such as Linux, SOlaris, Mac OS X etc.

The makefile assumes that GMT 4 is installed.

The file HOWTO.fix discusses what I learned in 2004 when trying to update a portion of
the coastline and adding some new lakes etc.
