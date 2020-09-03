.. index:: ! gmtconvert
.. include:: module_core_purpose.rst_

**********
gmtconvert
**********

|gmtconvert_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt convert** [ *table* ]
[ |-A| ]
[ |-C|\ [**+l**\ *min*][**+u**\ *max*][**+i**]]
[ |-D|\ [*template*\ [**+o**\ *orig*]] ]
[ |-E|\ [**f**\|\ **l**\|\ **m**\|\ **M**\ *stride*] ]
[ |-F|\ [**c**\|\ **n**\|\ **r**\|\ **v**][**a**\|\ **f**\|\ **s**\|\ **r**\|\ *refpoint*] ]
[ |-I|\ [**tsr**] ]
[ |-L| ]
[ |-N|\ *col*\ [**+a**\|\ **d**] ]
[ |-Q|\ [**~**]\ *selection*]
[ |-S|\ [**~**]\ *"search string"* \| |-S|\ [**~**]/\ *regexp*/[**i**] ]
[ |-T|\ [**h**][**d**\ [[**~**]\ *selection*]] ]
[ |SYN_OPT-V| ]
[ |-W|\ [**+n**] ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**convert** reads its standard input [or input files] and writes out
the desired information to standard output. It can do a combination of
nine tasks: (1) convert between binary and ASCII data tables, (2) paste
corresponding records from multiple files horizontally into a single
file, (3) extract a subset of the available columns, (4) only extract
segments whose header record matches a text pattern search, (5) only
list segment headers and no data records, (6) extract first and/or last
data record for each segment, (7) reverse the order of items on output,
(8) output only ranges of segment numbers, and (9) output only segments
whose record count matches criteria.  Input (and hence output) may have multiple
sub-headers, and ASCII tables may have regular headers as well.

Required Arguments
------------------

None

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. _-A:

**-A**
    The records from the input files should be pasted horizontally, not
    appended vertically [Default]. All files must have the same number
    of segments and number of rows per segment. Note for binary input,
    all the files you want to paste must have the same number of columns
    (as set with **-bi**); ASCII tables can have different number of columns.

.. _-C:

**-C**\ [**+l**\ *min*][**+u**\ *max*][**+i**]
    Only output segments whose number of records matches your given criteria:
    Append **+l**\ *min* to ensure all segment must have at least *min* records
    to be written to output [0], and append **+u**\ *max*  to ensure all segments
    must have at most *max* records to be written [inf].  You may append **+i**
    to invert the selection, i.e., only segments with record counts outside the
    given range will be output.

.. _-D:

**-D**\ [*template*\ [**+o**\ *orig*]]
    For multiple segment data, dump each segment to a separate output
    file [Default writes a multiple segment file to stdout]. Append a
    format template for the individual file names; this template
    **must** contain a C format specifier that can format an integer
    argument (the running segment number across all tables); this is
    usually %d but could be %08d which gives leading zeros, etc.
    [Default is gmtconvert_segment\_%d.{txt\|bin}, depending on
    **-bo**]. Append **+o**\ *orig* to start the numbering from *orig*
    instead of zero.  Alternatively, give a template with
    two C format specifiers and we will supply the table number and the
    segment number within the table to build the file name.
    Append **+o**\ *torig*\ /*sorig* to start the numbering of tables
    from *torig* and numbering of segments within a table from *sorig*
    instead of zero.  The **+o** modifier will be stripped off before
    the *template* is used.

.. _-E:

**-E**\ [**f**\|\ **l**\|\ **m**\|\ **M**\ *stride*]
    Only extract the first and last record for each segment of interest
    [Default extracts all records]. Optionally, append **f** or **l** to
    only extract the first or last record of each segment, respectively.
    Alternatively, append **m**\ *stride* to extract every *stride* records;
    use **M** to also include the last record.

.. _-F:

**-F**\ [**c**\|\ **n**\|\ **r**\|\ **v**][**a**\|\ **f**\|\ **s**\|\ **r**\|\ *refpoint*]
    Alter the way points are connected (by specifying a *scheme*) and data are grouped (by specifying a *method*).
    Append one of four line connection schemes:
    **c**\ : Form continuous line segments for each group [Default].
    **r**\ : Form line segments from a reference point reset for each group.
    **n**\ : Form networks of line segments between all points in each group.
    **v**\ : Form vector line segments suitable for :doc:`plot` **-Sv+s**.
    Optionally, append the one of four segmentation methods to define the group:
    **a**\ : Ignore all segment headers, i.e., let all points belong to a single group,
    and set group reference point to the very first point of the first file.
    **f**\ : Consider all data in each file to be a single separate group and
    reset the group reference point to the first point of each group.
    **s**\ : Segment headers are honored so each segment is a group; the group
    reference point is reset to the first point of each incoming segment [Default].
    **r**\ : Same as **s**, but the group reference point is reset after
    each record to the previous point (this method is only available with the **-Fr** scheme).
    Instead of the codes **a**\|\ **f**\|\ **s**\|\ **r** you may append
    the coordinates of a *refpoint* which will serve as a fixed external
    reference point for all groups.

.. _-I:

**-I**\ [**tsr**]
    Invert the order of items, i.e., output the items in reverse order,
    starting with the last and ending up with the first item [Default
    keeps original order]. Append up to three items that should be
    reversed: **t** will reverse the order of tables, **s** will reverse
    the order of segments within each table, and **r** will reverse the
    order of records within each segment [Default].

.. _-L:

**-L**
    Only output a listing of all segment header records and no data
    records (requires ASCII data).

.. _-N:

**-N**\ *col*\ [**+a**\|\ **d**]
    Numerically sort each segment based on values in column *col*.
    The data records will be sorted such that the chosen column will
    fall into ascending order [**+a**\ , which is Default].  Append **+d**
    to sort into descending order instead.  The **-N** option can be
    combined with any other ordering scheme except **-F** (segmentation)
    and is applied at the end.

.. _-Q:

**-Q**\ [**~**]\ *selection*
    Only write segments whose number is included in *selection* and skip
    all others. Cannot be used with **-S**. The *selection* syntax is
    *range*\ [,\ *range*,...] where each *range* of items is either a single
    segment *number* or a range with stepped increments given via *start*\ [:*step*:]\ :*stop*
    (*step* is optional and defaults to 1). A leading **~** will
    invert the selection and write all segments but the ones listed.  Instead
    of a list of ranges, use **+f**\ *file* to supply a file list with one *range* per line.

.. _-S:

**-S**\ [**~**]\ *"search string"* or **-S**\ [**~**]/\ *regexp*/[**i**]
    Only output those segments whose header record contains the
    specified text string. To reverse the search, i.e., to output
    segments whose headers do *not* contain the specified pattern, use
    **-S~**. Should your pattern happen to start with ~ you need to
    escape this character with a backslash [Default output all
    segments]. Cannot be used with **-Q**. For matching segments based
    on aspatial values (via OGR/GMT format), give the search string as
    *varname*\ =\ *value* and we will compare *value* against the value
    of *varname* for each segment. **Note**: If the features are polygons
    then a match of a particular polygon perimeter also means that any
    associated polygon holes will also be matched. For matching segment
    headers against extended regular expressions enclose the expression
    in slashes. Append **i** for case-insensitive matching.
    For a list of such patterns, give **+f**\ *file* with one pattern per line.
    To give a single pattern starting with +f, escape it with a backslash.

.. _-T:

**-T**\ [**h**][**d**\ [[**~**]\ *selection*]]
    Suppress the writing of certain records on output.  Append **h** to
    suppress segment headers [Default], and/or **d** to suppress duplicate
    data records.  Use **-Thd** to suppress both types of records.  By default,
    all columns must be identical across the two records to skip the record.
    ALternatively, append a column *selection* to only use those columns
    in the comparisons instead.  The *selection* syntax is
    *range*\ [,\ *range*,...] where each *range* of items is either a single
    column *number* or a range with stepped increments given via *start*\ [:*step*:]\ :*stop*
    (*step* is optional and defaults to 1). A leading **~** will
    invert the selection and select all columns but the ones listed. To add the
    trailing text to the comparison as well, add the column *t* to the list.
    If no numerical columns are specified, just *t*, then we only consider trailing text.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**+n**]
    Attempt to convert each word in the trailing text to a number and append
    such values to the numerical output columns.  Text that cannot be converted
    (because they are not numbers) will appear as NaNs.  Use modifier **+n** to
    exclude the columns with NaNs.  **Note**: These columns are identified based on
    the first input record only.

.. include:: explain_-aspatial.rst_

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_


Examples
--------

.. include:: explain_example.rst_

To convert the binary file test.b (single precision) with 4 columns to ASCII::

    gmt convert test.b -bi4f > test.dat

To convert the multiple segment ASCII table test.txt to a double precision binary file::

    gmt convert test.txt -bo > test.b

You have an ASCII table with 6 columns and you want to plot column 5 versus column 0. Try::

    gmt convert table.txt -o5,0 | gmt plot ...

If the file instead is the binary file results.b which has 9
single-precision values per record, we extract the last column and
columns 4-6 and write ASCII with the command::

    gmt convert results.b -o8,4-6 -bi9s | gmt plot ...

You want to plot the 2nd column of a 2-column file left.txt versus the
first column of a file right.txt::

    gmt convert left.txt right.txt -A -o1,2 | gmt plot ...

To extract all segments in the file big_file.txt whose headers contain
the string "RIDGE AXIS", try::

    gmt convert big_file.txt -S"RIDGE AXIS" > subset.txt

To invert the selection of segments whose headers begin with "profile "
followed by an integer number and any letter between "g" and "l", try::

    gmt convert -S~"/^profile [0-9]+[g-l]$/"

To reverse the order of segments in a file without reversing the order
of records within each segment, try::

    gmt convert lots_of_segments.txt -Is > last_segment_first.txt

To extract segments 20 to 40 in steps of 2, plus segment 0 in a file, try::

    gmt convert lots_of_segments.txt -Q0,20:2:40 > my_segments.txt


To extract the attribute ELEVATION from an ogr gmt file like this::

    # @VGMT1.0 @GPOINT
    ...
    # @NELEVATION|DISPX|DISPY
    # @Tdouble|double|double
    # FEATURE_DATA
    # @D4.945000|-106500.00000000|-32700.00000000
    -9.36890245902635 39.367156766570389

do::

    gmt convert file.gmt -a2=ELEVATION > xyz.dat

or just::

    gmt convert file.gmt -aELEVATION > xyz.dat

To connect all points in the file sensors.txt with the specified origin
at 23.5/19, try::

    gmt convert sensors.txt -F23.5/19 > lines.txt

To write all segments in the two files A.txt and B.txt to
individual files named profile_005000.txt, profile_005001.txt, etc.,
where we reset the origin of the sequential numbering from 0 to 5000, try::

    gmt convert A.txt B.txt -Dprofile_%6.6d.txt+o5000

To only read rows 100-200 and 500-600 from file junk.txt, try::

    gmt convert junk.txt -q100-200,500-600 < subset.txt

To get all rows except those bad ones between rows 1000-2000, try::

    gmt convert junk.txt -q~1000-2000 > good.txt

See Also
--------

:doc:`gmt`,
:doc:`gmtinfo`,
:doc:`gmtselect`
