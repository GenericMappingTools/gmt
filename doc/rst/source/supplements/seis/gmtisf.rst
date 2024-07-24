.. index:: ! gmtisf
.. include:: ../module_supplements_purpose.rst_

******
gmtisf
******

|gmtisf_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt gmtisf** *ISFfile*
|SYN_OPT-R|
[ |-D|\ *date_start*\ [/*date_end*] ]
[ |-F|\ [**a**] ]
[ |-N| ]
[ |SYN_OPT--| ]

Description
-----------

Reads seismicity data from an ISC (https://www.isc.ac.uk/iscbulletin) formated *file.isc* and output [lon lat depth mag ...]
to standard output.

Optional Arguments
------------------

.. _-R:

.. |Add_-Rgeo| replace:: |Add_-R_auto_table|
.. include:: ../../explain_-Rgeo.rst_

.. _-D:

**-D**\ *date_start*\ [/*date_end*]
    Limit the output locations to data hose date is >= date1, or between date1 and date2. <date> must be in ISO format, e.g, 2000-04-25"

.. _-F:

**-F**\ [*a*]
    Select only events that have focal mechanisms. The default is output in Global CMT convention. Append 'a' for the AKI convention.

    Focal mechanisms in Global CMT convention.

    **1**,\ **2**:
    longitude, latitude of event (**-:** option interchanges order)

    **3**:
    depth of event in kilometers

    **4**,\ **5**,\ **6**:
    strike, dip, and rake of plane 1

    **7**,\ **8**,\ **9**:
    strike, dip, and rake of plane 2

    **10**,\ **11**:
    mantissa and exponent of moment in dyne-cm


    Focal mechanisms in Aki and Richards convention.

    **1**,\ **2**:
    longitude, latitude of event (**-:** option interchanges order)

    **3**:
    depth of event in kilometers

    **4**,\ **5**,\ **6**:
    strike, dip and rake in degrees

    **7**:
    magnitude

.. _-N:

**-N**
      The default is to output time information [year month day hour minute] as the last 5 columns.
      Use this option to skip those last 5 columns.

Examples
--------

Extract the `lon lat depth mag` seismicity from file `file.isf` obtained at (https://www.isc.ac.uk/iscbulletin/)
and limiting over a geographic region and a date interval::

    gmt isf file.isf -R-15/0/30/45 -D2001-01-01/2005-12-31 -N > seismicity.dat

The above command can be piped directly to `psxy` to create a seismicity map. _e.,g._::

    gmt isf file.isf -R-15/0/30/45 -D2001-01-01/2005-12-31 -N | gmt psxy -R -JM14c -Sc0.01 -Gblack -Ba -P > seis.ps

See Also
--------

:doc:`psmeca`,
:doc:`pspolar`,
:doc:`pscoupe`,
:doc:`gmt </gmt>`, :doc:`psbasemap </psbasemap>`, :doc:`psxy </psxy>`
