.. index:: ! x2sys_solve

***********
x2sys_solve
***********

.. only:: not man

    x2sys_solve - Determine least-squares systematic correction from crossovers

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_solve** |-C|\ *column* |-T|\ *TAG* |-E|\ **c**\ \|\ **d**\ \|\ **g**\ \|\ **h**\ \|\ **s**\ \|\ **y**\ \|\ **z**
[ *COE_list.txt* ]
[ |SYN_OPT-V| ]
[ |-W|\ **[u]** ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_solve** will use the supplied crossover information to solve
for systematic corrections that can then be applied per track to improve
data quality. Seven different systematic corrections can be solved for using a
least-squares approach. Note: Only one data column can be processed at
the time.

Required Arguments
------------------

*COE_list.txt*
    Name of file with the required crossover columns as produced by
    :doc:`x2sys_list`. NOTE: If **-bi** is used
    then the first two columns are expected to hold the integer track
    IDs; otherwise we expect those columns to hold the text string names
    of the two tracks. If no file is given we will read from *stdin*.

.. include:: explain_tag.rst_

.. _-C:

**-C**\ *column*
    Specify which data column you want to process. Needed for proper
    formatting of the output correction table and must match the same
    option used in :doc:`x2sys_list` when preparing the input data.

.. _-E:

**-E**\ **c**\ \|\ **d**\ \|\ **g**\ \|\ **h**\ \|\ **s**\ \|\ **y**\ \|\ **z**
    The correction type you wish to model. Choose among the following
    functions f(*p*) , where *p* are the *m*
    parameters per track that we will fit simultaneously using a least
    squares approach:

    **c** will fit f(*p*) = *a* (a constant offset);
    records must contain track ID1, ID2, COE.

    **d** will fit f(*p*) = *a* + *b* \* *d* (linear
    drift; *d* is distance; records must contain track ID1, ID2, d1,
    d2, COE.

    **g** will fit f(*p*) = *a* + *b* sin(y)^2
    (1980-1930 gravity correction); records must contain track ID1,
    ID2, latitude y, COE.

    **h** will fit f(*p*) = *a* + *b* cos(H) + *c*
    cos(2H) + *d* sin(H) + *e* sin(2H)
    (magnetic heading correction); records must contain track ID1, ID2,
    heading H, COE.

    **s** will fit f(*p*) = *a* \* z (a unit scale
    correction); records must contain track ID1, ID2, z1, z2.

    **t** will fit f(*p*) = *a* + *b* \* (*t - t0*)
    (linear drift; *t0* is the start time of the track); records must
    contain track ID1, ID2, t1-t0, t2-t0, COE.

    **z** will fit f(*p*) = *a* + *b* \* z (an offset plus a unit scale
    correction); records must contain track ID1, ID2, z1, z2.


Optional Arguments
------------------

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**
    Means that each input records has an extra column with the composite
    weight for each crossover record. These are used to obtain a
    weighted least squares solution [no weights]. Append **u** to report
    unweighted mean/std [Default, report weighted stats].

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. include:: ../../explain_core.rst_

.. include:: ../../explain_help.rst_

Notes
-----

Most of the model corrections in |-E| involve a constant offset.
Because crossovers are differences between values, any absolute level
will cancel out and hence the constant offsets we obtain are relative
to an undetermined absolute level.  To obtain a solvable solution we
add the constraint that the sum of all constant offsets equal zero.
If the tracks form clusters in which no tracks from one cluster cross
any track from another cluster then these are two independent data
sets and require they own constraint equation for their offsets.  We
determine the number of clusters and automatically add the required
constraint equations.  If you need a particular reference track to have
a particular offset (e.g., 0) then you can subtract the offset you
found from every track correction and add in the desired offset.

Examples
--------

To fit a simple bias offset to faa for all tracks under the MGD77 tag, try

   ::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cfaa -Fnc > faa_coe.txt
    gmt x2sys_solve faa_coe.txt -V -TMGD77 -Cfaa -Ec > coe_table.txt

To fit a faa linear drift with time instead, try

   ::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cfaa -FnTc > faa_coe.txt
    gmt x2sys_solve faa_coe.txt -V -TMGD77 -Cfaa -Et > coe_table.txt

To estimate heading corrections based on magnetic crossovers associated
with the tag MGD77 from the file COE_data.txt, try

   ::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cmag -Fnhc > mag_coe.txt
    gmt x2sys_solve mag_coe.txt -V -TMGD77 -Cmag -Eh > coe_table.txt

To estimate unit scale corrections based on bathymetry crossovers, try

   ::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cdepth -Fnz > depth_coe.txt
    gmt x2sys_solve depth_coe.txt -V -TMGD77 -Cdepth -Es > coe_table.txt

See Also
--------

:doc:`x2sys_binlist`,
:doc:`x2sys_cross`,
:doc:`x2sys_datalist`,
:doc:`x2sys_get`,
:doc:`x2sys_init`,
:doc:`x2sys_list`,
:doc:`x2sys_put`,
:doc:`x2sys_report`
