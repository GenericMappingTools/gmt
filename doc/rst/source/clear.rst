.. index:: ! clear
.. include:: module_core_purpose.rst_

*****
clear
*****

|clear_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt clear** **all** \| **cache** \| **data**\ [=\ *planet*] \| **geography**\ [=\ *name*] \| **sessions** \| **settings**
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

The clear command allows users to delete their entire user cache, data, geography, or sessions
directories, and in modern mode their defaults settings (i.e., gmt.conf), or all of the above.

Optional Arguments
------------------

.. _clear-all:

**all**
    Deletes all the items under the control of the individual targets.

.. _clear-cache:

**cache**
    Delete the user's cache directory and all of its contents.

.. _clear-data:

**data**\ [=\ *planet*]
    Delete the user's data download server directory and all of its contents.
    Alternatively, append =\ *planet* for a specific planet and we only delete
    data for that sub-directory [all planets].

.. _clear-geography:

**geography**\ [=\ *name*]
    Delete the user's geography directory.  Append either *=gshhg* or *=dcw*
    if you only want to delete the named data set [both].

.. _clear-sessions:

**sessions**
    Delete the user's sessions directory.

.. _clear-settings:

**settings**
    Delete the current default settings file (gmt.conf) used for the current modern session.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_help_nopar.rst_

Examples
--------

To remove the current default settings in a modern mode session, use::

    gmt clear settings

To completely wipe your GMT cache directory, try::

    gmt clear cache

To remove your GMT geography directory with downloaded GSHHG and DCW data, try::

    gmt clear geography

To only wipe your GMT server directory for Earth data, try::

    gmt clear data=earth

To only wipe your entire GMT server and cache directories, (carefully) try::

    gmt clear all

.. include:: data-updating.rst_

See Also
--------

:doc:`begin`,
:doc:`docs`,
:doc:`end`,
:doc:`figure`,
:doc:`inset`,
:doc:`subplot`,
:doc:`gmt`
