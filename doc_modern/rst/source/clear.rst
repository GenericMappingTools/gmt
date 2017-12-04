.. index:: ! clear

*****
clear
*****

.. only:: not man

    Delete current history, conf, cpt, or the cache directory

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt clear** [*all* \|\ *cache* \|\ *conf* \|\ *cpt* \|\ *history* ] [ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

The clear command allows users to delete their current history, conf,
or cpt file, remove the entire user cache directory, or all of the above.

Optional Arguments
------------------

.. _clear-all:

*all*
    Deletes all the items under the control of the individual targets.

.. _clear-conf:

*conf*
    Delete the current gmt.conf file used for the modern session.

.. _clear-cpt:

*cpt*
    Delete the current (and hidden) cpt in effect (if any) for the modern session.

.. _clear-cache:

*cache*
    Delete the user's cache directory and its contents.

.. _clear-history:

*history*
    Deletes the current gmt.history file for the modern session.

.. _clear-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

Examples
--------

To remove the current CPT file that was created by grdimage in an
earlier command, use

   ::

    gmt clear cpt

To completely wipe your GMT cache directory, try

   ::

    gmt clear cache

See Also
--------

:doc:`begin`,
:doc:`end`,
:doc:`figure`,
:doc:`subplot`,
:doc:`gmt`
