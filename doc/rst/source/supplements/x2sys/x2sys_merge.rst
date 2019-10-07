.. index:: ! x2sys_merge

***********
x2sys_merge
***********

.. only:: not man

    x2sys_merge - Merge an updated COEs table (smaller) into the main table (bigger)

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_merge** |-A|\ *main_COElist.txt* |-M|\ *new_COElist.txt*
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_merge** will read two crossovers data base and output the
contents of the main one updated with the COEs in the second one. The
second file should only contain updated COEs relatively to the first
one. That is, it MUST NOT contain any new two tracks intersections (This
point is NOT checked in the code). This program is useful when, for any
good reason like file editing NAV correction or whatever, one had to
recompute only the COEs between the edited files and the rest of the
database.

Required Arguments
------------------

.. _-A:

**-A**\ *main_COElist.txt*
    Specify the file *main_COElist.txt* with the main crossover error data base.

.. _-M:

**-M**\ *new_COElist.txt*
    Specify the file *new_COElist.txt* with the newly computed crossover error data base.

Optional Arguments
------------------

Examples
--------

To update the main COE_data.txt with the new COEs estimations saved in
the smaller COE_fresh.txt, try

   ::

    gmt x2sys_merge -ACOE_data.txt -MCOE_fresh.txt > COE_updated.txt

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
