***********
x2sys_merge
***********

x2sys\_merge - Merge an updated COEs table (smaller) into the main table
(bigger)

`Synopsis <#toc1>`_
-------------------

**x2sys\_merge** **-A**\ *main\_COElist.d* **-M**\ *new\_COElist.d*

`Description <#toc2>`_
----------------------

**x2sys\_merge** will read two crossovers data base and output the
contents of the main one updated with the COEs in the second one. The
second file should only contain updated COEs relatively to the first
one. That is, it MUST NOT contain any new two tracks intersections (This
point is NOT checked in the code). This program is useful when, for any
good reason like file editing NAV correction or whatever, one had to
recompute only the COEs between the edited files and the rest of the
database.

.. include:: ../../explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

**-A**\ *main\_COElist.d*
    Specify the file *main\_COElist.d* with the main crossover error
    data base.
**-M**\ *new\_COElist.d*
    Specify the file *new\_COElist.d* with the newly computed crossover
    error data base.

`Optional Arguments <#toc5>`_
-----------------------------

`Examples <#toc6>`_
-------------------

To update the main COE\_data.txt with the new COEs estimations saved in
the smaller COE\_fresh.txt, try

    x2sys\_merge -ACOE\_data.txt -MCOE\_fresh.txt > COE\_updated.txt

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
