***********
x2sys_solve
***********

x2sys\_solve - Determine least-squares systematic correction from crossovers

`Synopsis <#toc1>`_
-------------------

**x2sys\_solve** **-C**\ *column* **-T**\ *TAG* **-E**\ *mode* [
*COE\_list.d* ] [ **-V**\ [*level*\ ] ] [ **-W[u]** ] [
**-bi**\ [*ncols*\ ][*type*\ ] ]

`Description <#toc2>`_
----------------------

**x2sys\_solve** will use the supplied crossover information to solve
for systematic corrections that can then be applied per track to improve
data quality. Several systematic corrections can be solved for using a
least-squares approach. Note: Only one data column can be processed at
the time.

.. include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

*COE\_list.d*
    Name of file with the required crossover columns as produced by
    **x2sys\_list**. NOTE: If **-bi**\ [*ncols*\ ][*type*\ ] is used
    then the first two columns are expected to hold the integer track
    IDs; otherwise we expect those columns to hold the text string names
    of the two tracks. If no file is given we will read from *stdin*.

.. include:: explain_tag.rst_

**-C**\ *column*
    Specify which data column you want to process. Needed for proper
    formatting of the output correction table and must match the same
    option used in **x2sys\_list** when preparing the input data.
**-E**\ *mode*
    The correction type you wish to model. Choose among the following
    functions `*f*\ (*p*) <f.p.html>`_ , where *p* are the *m*
    parameters per track that we will fit simultaneously using a least
    squares approach:

    **c** will fit `*f*\ (*p*) <f.p.html>`_ = *a* (a constant offset);
    records must contain cruise ID1, ID2, COE.

    **d** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* \* *d* (linear
    drift; *d* is distance; records must contain cruise ID1, ID2, d1,
    d2, COE.

    **g** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* sin(y)^2
    (1980-1930 gravity correction); records must contain cruise ID1,
    ID2, latitude y, COE.

    **h** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* cos(H) + *c*
    `cos(2H) <cos.2H.html>`_ + *d* sin(H) + *e* `sin(2H) <sin.2H.html>`_
    (magnetic heading correction); records must contain cruise ID1, ID2,
    heading H, COE.

    **s** will fit `*f*\ (*p*) <f.p.html>`_ = *a* \* z (a unit scale
    correction); records must contain cruise ID1, ID2, z1, z2.

    **t** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* \* (*t - t0*)
    (linear drift; *t0* is the start time of the track); records must
    contain cruise ID1, ID2, t1-t0, t2-t0, COE.

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_-V| unicode:: 0x0C .. just an invisible code
.. include:: ../../explain_-V.rst_

**-W**
    Means that each input records has an extra column with the composite
    weight for each crossover record. These are used to obtain a
    weighted least squares solution [no weights]. Append ’u’ to report
    unweighted mean/std [Default, report weighted stats].

.. |Add_-bi| unicode:: 0x0C .. just an invisible code
.. include:: ../../explain_-bi.rst_

.. include:: ../../explain_help.rst_

`Examples <#toc6>`_
-------------------

To fit a simple bias offset to faa for all tracks under the MGD77 tag, try

x2sys\_list COE\_data.txt -V -TMGD77 -Cfaa -Fnc > faa\_coe.txt

x2sys\_solve faa\_coe.txt -V -TMGD77 -Cfaa -Ec > coe\_table.txt

To fit a faa linear drift with time instead, try

x2sys\_list COE\_data.txt -V -TMGD77 -Cfaa -FnTc > faa\_coe.txt

x2sys\_solve faa\_coe.txt -V -TMGD77 -Cfaa -Et > coe\_table.txt

To estimate heading corrections based on magnetic crossovers associated
with the tag MGD77 from the file COE\_data.txt, try

x2sys\_list COE\_data.txt -V -TMGD77 -Cmag -Fnhc > mag\_coe.txt

x2sys\_solve mag\_coe.txt -V -TMGD77 -Cmag -Eh > coe\_table.txt

To estimate unit scale corrections based on bathymetry crossovers, try

x2sys\_list COE\_data.txt -V -TMGD77 -Cdepth -Fnz > depth\_coe.txt

x2sys\_solve depth\_coe.txt -V -TMGD77 -Cdepth -Es > coe\_table.txt

`See Also <#toc7>`_
-------------------

`x2sys\_binlist <x2sys_binlist.html>`_ ,
`x2sys\_cross <x2sys_cross.html>`_ ,
`x2sys\_datalist <x2sys_datalist.html>`_ ,
`x2sys\_get <x2sys_get.html>`_ ,
`x2sys\_init <x2sys_init.html>`_ ,
`x2sys\_list <x2sys_list.html>`_ ,
`x2sys\_put <x2sys_put.html>`_ ,
`x2sys\_report <x2sys_report.html>`_
