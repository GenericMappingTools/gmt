.. index:: ! begin
.. include:: module_core_purpose.rst_

*****
begin
*****

|begin_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt begin** [*prefix*] [*formats*] [*options*]
[ |-C| ]
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

The **begin** module instructs GMT to begin a new modern mode session.  If your script only makes
a single plot then this is the most opportune time to specify the name
and format(s) of your plot (see also :doc:`cookbook/one-liner`).
However, if you want to create multiple illustrations within this session,
you will instead use :doc:`figure` to name the figure(s) you wish to make.  The session
keeps track of all default and history settings and isolates them from any other session
that may run concurrently.  Thus, unlike classic mode, you can run multiple modern sessions
simultaneously without having destructive interference in updating the history of common
options.
In addition to *prefix* and *formats*, you can supply a comma-separated series of
:doc:`psconvert` *options* (without their leading hyphens) that will override the default settings provided via
:term:`PS_CONVERT` [**A**]. The only other available options control the verbosity.

Optional Arguments
------------------

.. _begin-prefix:

*prefix*
    Name-stem used to construct the single final figure name [gmtsession].  The extension is appended
    automatically from your *formats* selection(s).  If your script only
    performs calculations or needs to make several figures then you will not use this argument.
    While not recommended, if your *prefix* has spaces in it then you must enclose your
    prefix in single or double quotes. You may also include a relative or absolute output path.

.. _begin-formats:

*formats*
    Give one or more comma-separated graphics extensions from the list of allowable
    :ref:`graphics formats <tbl-formats>`
    (default format is configurable via setting :term:`GMT_GRAPHICS_FORMAT` [pdf]).
    Optionally, append **+m** for monochrome image (BMP, JPEG, PNG, and TIFF only)
    and **+q**\ *quality* in 0-100 range to change JPEG quality [90].
    If you specify one or more formats, you should also supply a :ref:`prefix <begin-prefix>`.

.. _begin-options:

*options*
    Sets one or more comma-separated options (and possibly arguments) that
    can be passed to :doc:`psconvert` when preparing a session figure [**A**].
    The valid subset of options are
    **A**\ [*args*],\ **C**\ *args*,\ **D**\ *dir*,\ **E**\ *dpi*,\ **H**\ *factor*,\ **I**\ *args*,\ **M**\ *args*,\ **N**\ *args*,\ **Q**\ *args*,\ **S**.
    Note that the leading hyphens should not be given.
    See the :doc:`psconvert` documentation for details on these options.

.. _-C:

**-C**
    Start this session with a clean slate: Any gmt.conf files in the usual search path
    directories are ignored [Default starts session with the prevailing user settings].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_help_nopar.rst_


Supported Graphic Formats
-------------------------

.. _tbl-formats:

====== ====================================================
Format Explanation
====== ====================================================
bmp    Microsoft Bit Map
eps    Encapsulated PostScript
jpg    Joint Photographic Experts Group Format
pdf    Portable Document Format [Default]
png    Portable Network Graphics
PNG    Portable Network Graphics (with transparency layer)
ppm    Portable Pixel Map
ps     Plain PostScript
tif    Tagged Image Format File
view   Use format set by GMT_GRAPHICS_FORMAT
====== ====================================================

Examples
--------

To initiate a new modern session that will produce a single
map called Figure_2 saved as both a PDF vector graphics file
and an opaque PNG raster image, we would start our script thus::

    gmt begin Figure_2 pdf,png

If the modern session is only used for computations and no illustrations
are produced then we do not need to give any further arguments::

    gmt begin

Should we give such a command and still produce a plot then it will automatically
be called gmtsession.pdf (assuming :term:`GMT_GRAPHICS_FORMAT` is pdf).

To set up proceedings for a jpg figure with 0.5c white margin, and strictly using
the GMT default settings, we would run::

    gmt begin 'My Figure4' jpg A,I+m0.5c -C

.. include:: explain_postscript.rst_

Note on UNIX shells
-------------------

Modern mode works by communicating across gmt modules via the shell script's (or terminal's)
process ID, which is the common parent process ID (PPID) for each module.  This number is used to
create the unique session directories where gmt keeps its book-keeping records.  However, inconsistencies
across various UNIX shells and other differences in their implementations may occasionally lead
to problems for gmt to properly determine the unique PPID.  The most common situation is
related to a shell spawning sub-shells when you are linking two or more processes via UNIX pipes.
Each sub-shell will then have its own process ID and gmt modules started by the sub-shell will then
have that ID as PPID and it will differ from the one determined by gmt begin.
If you are using pipes in your modern mode script and you get strange errors about not finding gmt_session.#####
then you can add this command to the top of your script to make the issue go away (in Bourne shell)::

    export GMT_SESSION_NAME=$$

or in C shell::

    setenv GMT_SESSION_NAME $$

This setting is prescribed if you create a new script with ``gmt --new-script``, as is the **-e** option
that will stop the script if any command returns an error.

Because of this mode of communication you can also not run two separate modern mode scripts
from the same terminal at the same time (e.g., job_1.sh &; job_2.sh &) since they would share the
same GMT_SESSION_NAME (unless you reassigned it explicitly in the scripts).  Finally, if you
Ctrl-C a modern mode command it will first try to remove the hidden gmt_session.###### directory.
Should you try to terminate a script with a mix of GMT and UNIX commands then whatever
process is running when you hit Ctrl-C will be the one that stops, and if that is not a GMT command
then the hidden directory will be left behind.  You can clean this up via::

    gmt clear sessions

See Also
--------

:doc:`clear`,
:doc:`docs`,
:doc:`end`,
:doc:`figure`,
:doc:`inset`,
:doc:`subplot`,
:doc:`gmt`
