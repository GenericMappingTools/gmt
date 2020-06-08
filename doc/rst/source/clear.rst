.. index:: ! clear
.. include:: module_core_purpose.rst_

*****
clear
*****

|clear_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt clear** **all** \| **cache** \| **data**\ [=\ *planet*] \| **sessions** \| **settings**
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

.. _clear-cache:

**cache**
    Delete the user's cache directory and all of its contents.

.. _clear-data:

**data**\ [=\ *planet*]
    Delete the user's data download server directory and all of its contents.
    Alternatively, append =\ *planet* for a specific planet and we only delete
    data for that sub-directory [all planets].

.. _clear-sessions:

**sessions**
    Delete the user's sessions directory.

.. _clear-settings:

**settings**
    Delete the current default settings file (gmt.conf) used for the current modern session.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help_nopar.rst_

Examples
--------

To remove the current default settings in a modern mode session, use::

    gmt clear settings

To completely wipe your GMT cache directory, try::

    gmt clear cache

To only wipe your GMT server directory for Earth data, try::

    gmt clear data=earth

To only wipe your entire GMT server and cache directories, (carefully) try::

    gmt clear all

Data and Cache Updates
----------------------

Remote datasets and remote cache files are updated from time to time as our aim is to present the very latest version
of the data.  This means we (a) do not support multiple versions of a dataset (you must obtain
earlier versions of any published data sets elsewhere), (b) as GMT detects a new version on the
server it will download it when you access the remote data set and overwrite any previous version,
and (c) any data set or file we deem obsolete will be removed from the server, and hence will be removed
from your .gmt/server area as well when the syncing occurs.  This is particularly true of the cache data
that may come and go.  If there are any important data to you in the cache listing we recommend you
place a copy in another place. Syncing occurs at most once a day.


See Also
--------

:doc:`begin`,
:doc:`docs`,
:doc:`end`,
:doc:`figure`,
:doc:`inset`,
:doc:`subplot`,
:doc:`gmt`
