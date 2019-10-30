.. index:: ! clear
.. include:: module_core_purpose.rst_

*****
clear
*****

|clear_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt clear** **all** \| **cache** \| **defaults** \| **data** \| **sessions**
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

The clear command allows users to delete their entire user cache, data, or sessions
directories, and in modern mode their defaults settings (i.e., gmt.conf), or all of the above.

Optional Arguments
------------------

.. _clear-all:

**all**
    Deletes all the items under the control of the individual targets.

.. _clear-defaults:

**defaults**
    Delete the current defaults (gmt.conf) file used for the current modern session.

.. _clear-cache:

**cache**
    Delete the user's cache directory and all of its contents.

.. _clear-data:

**data**
    Delete the user's data download directory and all of its contents.

.. _clear-sessions:

**sessions**
    Delete the user's sessions directory.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help_nopar.rst_

Examples
--------

To remove the current default settings in a modern mode session, use::

    gmt clear defaults

To completely wipe your GMT cache directory, try::

    gmt clear cache

See Also
--------

:doc:`begin`,
:doc:`docs`,
:doc:`end`,
:doc:`figure`,
:doc:`inset`,
:doc:`subplot`,
:doc:`gmt`
