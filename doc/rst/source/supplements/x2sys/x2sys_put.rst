*********
x2sys_put
*********

x2sys\_put - Update track index database from track bin file

`Synopsis <#toc1>`_
-------------------

**x2sys\_put** [ *info.tbf* ] **-T**\ *TAG* [ **-D** ] [ **-F** ] [
**-V**\ [*level*\ ] ]

`Description <#toc2>`_
----------------------

**x2sys\_put** accepts a track bin-index file created by
**x2sys\_binlist** and adds this information about the data tracks to
the relevant data base. You may chose to overwrite existing data with
new information for older tracks (**-F**) and even completely remove
information for certain tracks (**-D**). The x2sys *TAG* must match the
tag encoded in the *info.tbf* file. To inquire about tracks in the data
base, use **x2sys\_get**.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*info.tbf*
    Name of a single track bin file. If not given, *stdin* will be read.
**-T**\ *TAG*
    Specify the x2sys *TAG* which tracks the attributes of this data
    type.

`Optional Arguments <#toc5>`_
-----------------------------

**-D**
    Delete all tracks found in the track bin file [Default will try to
    add them as new track entries].
**-F**
    Replace any existing database information for these tracks with the
    new information in the track bin file [Default refuses to process
    tracks already in the database].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

To add the information stored in the track bin-index file latest.tbf to
the track data bases associated with the tag MGD77, and replace any
exiting information for these tracks, try

x2sys\_put latest.tbf -F -V -TMGD77

`X2sys Databases <#toc7>`_
--------------------------

The **x2sys\_put** utility adds new information to the x2sys data bases.
These consists of two files: The first file contains a listing of all
the tracks that have been added to the system; it is named
*TAG*\ \_tracks.d and is in ASCII format. The second file is named
*TAG*\ \_index.b and is in native binary format. It contains information
on which tracks cross each of the bins, and what data sets were observed
while crossing the bin. The bins are defined by the **-R** and **-I**
options passed to **x2sys\_init** when the *TAG* was first initiated.
Both data base files are stored in the **$X2SYS\_HOME**/*TAG* directory.
Do not attempt to edit these files by hand.

`See Also <#toc8>`_
-------------------

`*x2sys\_binlist*\ (1) <x2sys_binlist.html>`_ ,
`*x2sys\_get*\ (1) <x2sys_get.html>`_
