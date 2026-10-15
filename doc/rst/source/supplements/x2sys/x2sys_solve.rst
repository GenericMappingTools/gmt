.. index:: ! x2sys_solve
.. include:: ../module_supplements_purpose.rst_

***********
x2sys_solve
***********

|x2sys_solve_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_solve** |-C|\ *column* |-T|\ *TAG* |-E|\ **c**\|\ **d**\|\ **g**\|\ **h**\|\ **s**\|\ **t**\|\ **z**\ [**+r**\ [*K*]]
[ *COE_list.txt* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**+u**] ]
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
least-squares approach. **Note**: Only one data column can be processed at
the time.

Required Arguments
------------------

*COE_list.txt*
    Name of file with the required crossover columns as produced by
    :doc:`x2sys_list`. NOTE: If **-bi** is used
    then the first two columns are expected to hold the integer track
    IDs; otherwise we expect the trailing text to hold the text string names
    of the two tracks. If no file is given we will read from standard input.

.. include:: explain_tag.rst_

.. _-C:

**-C**\ *column*
    Specify which data column you want to process. Needed for proper
    formatting of the output correction table and must match the same
    option used in :doc:`x2sys_list` when preparing the input data.

.. _-E:

**-E**\ **c**\|\ **d**\|\ **g**\|\ **h**\|\ **s**\|\ **t**\|\ **z**\ [**+r**\ [*K*]]
    The correction type you wish to model. Choose among the following
    functions f(**p**) , where **p** are the *m*
    parameters per track that we will fit simultaneously using a least
    squares approach.  Each type implies a certain input data record
    format:

    **c** will fit f(**p**) = *a* (a constant offset);
    records must contain track COE, ID1, ID2.

    **d** will fit f(**p**) = *a* + *b* \* *d* (linear
    drift; *d* is distance along track; records must contain d1, d2, COE, ID1, ID2.

    **g** will fit f(**p**) = *a* + *b* sin(y)\ :sup:`2`
    (1980-1930 gravity correction); records must contain crossing latitude y, COE, ID1, ID2.

    **h** will fit f(**p**) = *a* + *b* cos(h) + *c*
    cos(2h) + *d* sin(h) + *e* sin(2h)
    (magnetic heading correction); *h* is heading at crossover; records must contain headings h1, h2, COE, ID1, ID2.

    **s** will fit f(**p**) = *a* \* z (a unit scale
    correction); *z* is the data value at the crossover; records must contain z1, z2, ID1, ID2.

    **t** will fit f(**p**) = *a* + *b* \* (*t - t0*)
    (linear drift; *t - t0* is the time along the track since start of track at *t0*); records must
    contain t1-t0, t2-t0, COE, D1, ID2.

    **z** will fit f(**p**) = *a* + *b* \* z (an offset plus a unit scale
    correction); *z* is the data value at the crossover; records must contain z1, z2, ID1, ID2.

    Append **+r**\ [*K*] to **d** or **t** to ridge-regularize the solve
    (see the Warning further below): this pulls a poorly-constrained
    track's offset back towards a plain, well-behaved correction instead
    of letting it blow up, while leaving well-constrained tracks nearly
    unaffected. *K* is a trust multiplier: the prior belief is that
    |offset| should not need to exceed roughly *K* times the
    pre-correction crossover-error standard deviation [10].


Optional Arguments
------------------

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [**+u**]
    Means that each input records has an extra column just before the ID columns
    with the composite weight for each crossover record. These are used to obtain a
    weighted least squares solution [no weights]. Append **+u** to report
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

**Warning**: Models that solve for more than one parameter per track
(**d**, **t**, **h**, **g**, **z**) need each track to have not just
at least as many crossings as parameters, but *enough of them, well
distributed along the track*, to separate the parameters reliably. A
track whose crossings are few and/or clustered together leaves its
part of the normal-equation system poorly conditioned. Because that
system is not exactly singular no error is raised, and the
least-squares solution can come back with an offset many times larger
than the crossover errors it was supposed to remove -- silently
corrupting that track instead of correcting it, even though the fit
still looks fine (crossover residuals stay small). **x2sys_solve**
flags such tracks with a warning naming the track, the ratio of its
solved offset to the pre-correction crossover-error scale, and how
many crossings back it. If you see this warning, inspect the flagged
track's connectivity (:doc:`x2sys_report` reports crossings per
track); try appending **+r** to **-Ed** or **-Et** to regularize the
solve (see |-E| above), or fall back to **-Ec** if your survey does
not have enough well-spread crossings per track to support a reliable
drift (or other multi-parameter) estimate.

Input Format
------------

In moving to a more robust data record definition in GMT 6, all text
items are now placed after the numerical columns.  For **x2sys_solve**, this
means that whereas the *ID1, ID2* track ids used to be expected in the first two
columns, they are now expected at the end.  Thus, you cannot use this module with
crossover tables produced by an earlier GMT version without reformatting.

Examples
--------

To fit a simple bias offset to faa for all tracks under the MGD77 tag, try::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cfaa -Fnc > faa_coe.txt
    gmt x2sys_solve faa_coe.txt -V -TMGD77 -Cfaa -Ec > coe_table.txt

To fit a faa linear drift with time instead, try::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cfaa -FnTc > faa_coe.txt
    gmt x2sys_solve faa_coe.txt -V -TMGD77 -Cfaa -Et > coe_table.txt

To estimate heading corrections based on magnetic crossovers associated
with the tag MGD77 from the file COE_data.txt, try::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cmag -Fnhc > mag_coe.txt
    gmt x2sys_solve mag_coe.txt -V -TMGD77 -Cmag -Eh > coe_table.txt

To estimate unit scale corrections based on bathymetry crossovers, try::

    gmt x2sys_list COE_data.txt -V -TMGD77 -Cdepth -Fnz > depth_coe.txt
    gmt x2sys_solve depth_coe.txt -V -TMGD77 -Cdepth -Es > coe_table.txt

.. include:: x2sys_refs.rst_

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
