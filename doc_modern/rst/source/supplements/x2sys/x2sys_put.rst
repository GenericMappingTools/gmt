.. index:: ! x2sys_put

*********
x2sys_put
*********

.. only:: not man

    x2sys_put - Update track index database from track bin file

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_put** [ *info.tbf* ] |-T|\ *TAG* [ |-D| ] [ |-F| ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_put** accepts a track bin-index file created by
:doc:`x2sys_binlist` and adds this information about the data tracks to
the relevant data base. You may chose to overwrite existing data with
new information for older tracks (**-F**) and even completely remove
information for certain tracks (**-D**). The x2sys *TAG* must match the
tag encoded in the *info.tbf* file. To inquire about tracks in the data
base, use :doc:`x2sys_get`.

Required Arguments
------------------

*info.tbf*
    Name of a single track bin file. If not given, *stdin* will be read.

.. include:: explain_tag.rst_

Optional Arguments
------------------

.. _-D:

**-D**
    Delete all tracks found in the track bin file [Default will try to
    add them as new track entries].

.. _-F:

**-F**
    Replace any existing database information for these tracks with the
    new information in the track bin file [Default refuses to process
    tracks already in the database].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To add the information stored in the track bin-index file latest.tbf to
the track data bases associated with the tag MGD77, and replace any
exiting information for these tracks, try

   ::

    gmt x2sys_put latest.tbf -F -V -TMGD77

X2sys Databases
---------------

The **x2sys_put** utility adds new information to the x2sys data bases.
These consists of two files: The first file contains a listing of all
the tracks that have been added to the system; it is named
*TAG*\ \_tracks.d and is in ASCII format. The second file is named
*TAG*\ \_index.b and is in native binary format. It contains information
on which tracks cross each of the bins, and what data sets were observed
while crossing the bin. The bins are defined by the |-R| and |-I|
options passed to :doc:`x2sys_init` when the *TAG* was first initiated.
Both data base files are stored in the **$X2SYS_HOME**/*TAG* directory.
Do not attempt to edit these files by hand.

See Also
--------

:doc:`x2sys_binlist`, :doc:`x2sys_get`
