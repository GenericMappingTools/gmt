.. index:: ! gmt_shell_functions.sh

**********************
gmt_shell_functions.sh
**********************

.. only:: not man

    gmt_shell_functions.sh - Practical functions to be used in GMT Bourne Again shell scripts

Synopsis
--------

**gmt_init_tmpdir**

**gmt_remove_tmpdir**

**gmt_clean_up** [ *prefix* ]

**gmt_message** *message*

**gmt_abort** *message*

**gmt_build_movie** [ **-d** *directory* ] [ **-n** ] [ **-r** *framerate* ] [ **-v** ] *namestem*

**gmt_build_gif** [ **-d** *directory* ] [ **-l** *loop* ] [ **-r** *delay* ] *namestem*

**gmt_build_kmz** **-p** *prefix* [ **-r** ] *files*

**gmt_get_nrecords** *file(s)*

**gmt_get_ndatarecords** *file(s)*

**gmt_get_nfields** *string*

**gmt_get_field** *string*

**gmt_get_region** *file(s)* [ *options* ]

**gmt_get_gridregion** *file* [ *options* ]

**gmt_get_map_width** **-R** **-J**

**gmt_get_map_height** **-R** **-J**

**gmt_movie_script** [ **-c** *canvas* OR **-e** *dpi* **-h** *height* **-w** *width* ] [ **-f** *format* ]
	[ **-g** *fill* ] [ **-n** *frames* ] [ **-m** *margin* ] [ **-r** *rate* ] *namestem*

**gmt_launch_jobs** [ **-c** *n_cores* ] [ **-l** *nlines_per_cluster* ] [ **-n** ] [ **-v** ] [ **-w** ] *commandfile*

**gmt_set_psfile** *scriptfile*

**gmt_set_pdffile** *scriptfile*

**gmt_set_framename** *prefix framenumber*

**gmt_set_framenext** *framenumber*

Description
-----------

**gmt_shell_functions.sh** provides a set of functions to Bourne
(again) shell scripts in support of GMT. The calling shell script
should include the following line, before the functions can be used:

**. gmt_shell_functions.sh**

Once included in a shell script, **gmt_shell_functions.sh** allows
GMT users to do some scripting more easily than otherwise. The
functions made available are:

**gmt_init_tmpdir**
    Creates a temporary directory in **/tmp** or (when defined) in the
    directory specified by the environment variable **TMPDIR**. The name
    of the temporary directory is returned as environment variable
    **GMT_TMPDIR**. This function also causes GMT to run in
    'isolation mode', i.e., all temporary files will be created in
    **GMT_TMPDIR** and the :doc:`gmt.conf` file will not be adjusted.

**gmt_remove_tmpdir**
    Removes the temporary directory and unsets the **GMT_TMPDIR**
    environment variable.

**gmt_cleanup**
    Remove all files and directories in which the current process number
    is part of the file name. If the optional *prefix* is given then we
    also delete all files and directories that begins with the given prefix.

**gmt_message**
    Send a message to standard error.

**gmt_abort**
    Send a message to standard error and exit the shell.

**gmt_get_nrecords**
    Returns the total number of lines in *file(s)*

**gmt_get_ndatarecords**
    Returns the total number of data records in *file(s)*, i.e., not counting headers.

**gmt_get_nfields**
    Returns the number of fields or words in *string*

**gmt_get_field**
    Returns the given *field* in a *string*. Must pass *string* between
    double quotes to preserve it as one item.

**gmt_get_region**
    Returns the region in the form w/e/s/n based on the data in table
    *file(s)*. Optionally add **-I**\ *dx*/\ *dy* to round off the answer.

**gmt_get_gridregion**
    Returns the region in the form w/e/s/n based on the header of a grid
    *file*. Optionally add **-I**\ *dx*/\ *dy* to round off the answer.

**gmt_get_map_width**
    Expects the user to give the desired **-R** **-J** settings and
    returns the map width in the current measurement unit.

**gmt_get_map_height**
    Expects the user to give the desired **-R** **-J** settings and
    returns the map height in the current measurement unit.

**gmt_movie_script**
    Creates an animation bash script template based on the arguments
    that set size, number of frames, video format etc. 
    Without arguments the function will display its usage.

**gmt_launch_jobs**
    Takes a file with a long list of commands and splits them into
    many chunks that can be executed concurrently. Without arguments
    the function will display its usage.  Note: It is your responsibility
    to make sure no race conditions occur (i.e., multiple commands
    writing to the same file).

**gmt_set_psfile**
    Create the output PostScript file name based on the base name of a
    given file (usually the script name **$0**).

**gmt_set_framename**
    Returns a lexically ordered filename stem (i.e., no extension) given
    the file prefix and the current frame number, using a width of 6 for
    the integer including leading zeros. Useful when creating animations
    and lexically sorted filenames are required.

**gmt_set_framenext**
    Accepts the current frame integer counter and returns the next
    integer counter.

**gmt_build_movie**
    Accepts a *namestem* which gives the prefix of a series of image files
    with names *dir*/*namestem*\ _*.*.  Optional argument sets the
    directory [same as *namestem* ], and frame rate [24].
    Without arguments the function will display its usage.

**gmt_build_gif**
    Accepts a *namestem* which gives the prefix of a series of image files
    with names *dir*/*namestem*\ _*.*.  Optional argument sets the
    directory [same as *namestem*], loop count and frame rate [24].
    Without arguments the function will display its usage.

**gmt_build_kmz**
    Accepts **-p** *prefix* [ **-r** ] and any number of KML files and
    and the images they may refer to, and builds a single KMZ file with
    the name *prefix*.kmz.
    Without arguments the function will display its usage.

Notes
-----

1. These functions only work in the Bourne shell (**sh**) and their
derivatives (like **ash**, **bash**, **ksh** and **zsh**). These
functions do not work in the C shell (**csh**) or their derivatives
(like **tcsh**), and cannot be used in DOS batch scripts either.

2. **gmt_shell_functions.sh** were first introduced in GMT version
4.2.2 and have since been regularly expanded with other practical
scripting short-cuts. If you want to suggest other functions, please do
so by adding a New Issue request on gmt.soest.hawaii.edu.

See Also
--------

:doc:`gmt` , :doc:`gmt.conf` ,
:doc:`gmtinfo` , :doc:`grdinfo`
