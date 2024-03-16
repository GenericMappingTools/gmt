.. index:: ! grdredpol
.. include:: ../module_supplements_purpose.rst_

*********
grdredpol
*********

|grdredpol_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt grdredpol** *ingrid*
|-G|\ *rtp_grd*
[ |-C|\ *dec/dip* ]
[ |-E|\ **i**\ *inc_grd* ]
[ |-E|\ **d**\ *dec_grd* ]
[ |-F|\ *m/n* ]
[ |-M|\ **m**\|\ **r** ]
[ |-N| ]
[ |-W|\ *win_width* ]
[ |SYN_OPT-V| ]
[ |-T|\ *year* ]
[ |-Z|\ *filtergrd* ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdredpol** will take a *.nc* file with a magnetic anomaly and compute
the reduction to the pole (RTP) anomaly. This anomaly is the one that
would have been produce if the bodies were magnetized vertically and the
anomalies were observed at the geomagnetic pole. Standard RTP procedure
assumes the direction of magnetization to be uniform throughout the
causative body, and the geomagnetic field to be uniform in direction
throughout the study region. Although these assumptions are reasonable
for small areas, they do not hold for large areas.

In the method used here computations are carried out in both the
frequency and the space domains. The idea is that a large area may be
decomposed in small size windows where both the ambient field and the
magnetization vector change by a very small amount. Inside each of those
windows, or bins, a set of filter coefficients are calculate and
reconstruct for each individual point the component filter using a first
order Taylor series expansion.

Required Arguments
------------------

.. |Add_ingrid| replace:: The anomaly grid to be converted.
.. include:: /explain_grd_inout.rst_
    :start-after: ingrid-syntax-begins
    :end-before: ingrid-syntax-ends

.. _-G:

**-G**\ *rtp_grd*
    is the filename for output grdfile with the RTP solution

Optional Arguments
------------------

.. _-C:

**-C**\ *dec/dip*
    Use this (constant) declination and inclination angles for both
    field and magnetization. This option consists in the classical RTP
    procedure.

.. _-E:

**-Ei**\ *inc_grd* **-Ed**\ *dec_grd*
    Get magnetization *INCLINATION* and *DECLINATION* from these grids
    [Default uses IGRF for each of the above parameters not provided via grid].
    **Note**: These two grids do not need to have the same resolution as
    the anomaly grid. They can be coarser.

.. _-F:

**-F**\ *m/n*
    The filter window size in terms of row/columns [Default value is 25/25].

.. _-M:

**-M**\ **m**\|\ **r**
    Set the boundary conditions. Directives **m**\|\ **r** stands for mirror or replicate edges
    [Default is zero padding].

.. _-N:

**-N**
    Do **not** use Taylor expansion.

.. _-R:

**-R**\ *west*/*east*/*south*/*north*
    defines the Region of the output points [Default: Same as input].

.. _-T:

**-T**\ *year*
    Decimal year used by the IGRF routine to compute the declination and
    inclination at each point [Default is 2000].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *width*
    The size of the moving window in degrees [5].

.. _-Z:

**-Z**\ *filter_grd*
    Write the filter file to disk.

.. include:: ../../explain_-n.rst_
.. include:: ../../explain_grdresample2.rst_

Examples
--------

Suppose that *anom.grd* is a file with the magnetic anomaly reduced to
the 2010 epoch and that the *dec.grd* and *dip.grd* contain the
magnetization declination and inclination respectively for an area that
encloses that of the *anom.grd*, compute the *RTP* using bins of 2
degrees and a filter of 45 coefficients.


::

  gmt grdredpol anom.grd -Grtp.grd -W2 -F45/45 -T2010 -Edec.grd/dip.grd -V

To compute the same *RTP* but now with the field and magnetization
vectors collinear and computed from IGRF :


::

  gmt grdredpol anom.grd -Grtp.grd -W2 -F45/45 -T2010 -V

Reference
---------

Luis, J.L. and Miranda, J.M., 2008, Reevaluation of magnetic chrons in
the North Atlantic between 35N and 47N: Implications for the formation
of the Azores Triple Junction and associated plateau. *JGR*, VOL.
**113**, B10105, doi:10.1029/2007JB005573
