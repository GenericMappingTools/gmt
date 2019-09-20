.. _GMT Overview and Quick Reference:

GMT Overview and Quick Reference
================================

GMT summary
-----------

The following is a summary of all the programs supplied with GMT and
a very short description of their purpose. For more details, see the
individual UNIX manual pages or the online web documentation. For a
listing sorted by program purpose, see Section `GMT quick reference`_.

.. include:: ../summary.rst_

GMT quick reference
-------------------

Instead of an alphabetical listing, this section
contains a summary sorted by program purpose. Also included is a quick
summary of the standard command line options and a breakdown of the
**-J** option for each of the over 30 projections available in GMT.

.. include:: ../quick_ref.rst_


GMT offers 31 map projections. These are specified using the **-J**
common option. There are two conventions you may use: (a) GMT-style
syntax and (b) **Proj4**\ -style syntax. The projection codes for the
GMT-style and the **Proj4**-style are tabulated below.

.. Substitution definitions:
.. |lon0| replace:: lon\ :sub:`0`
.. |lat0| replace:: lat\ :sub:`0`
.. |lon1| replace:: lon\ :sub:`1`
.. |lat1| replace:: lat\ :sub:`1`
.. |lat2| replace:: lat\ :sub:`2`
.. |lonp| replace:: lon\ :sub:`p`
.. |latp| replace:: lat\ :sub:`p`

.. include:: ../proj_codes.rst_

Finally, the rest of the GMT common options are given below:

.. include:: ../std_opts.rst_
