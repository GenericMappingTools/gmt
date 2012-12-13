*******************
gmt_shell_functions
*******************

gmt\_shell\_functions.sh - Practical functions to be used in GMT bourne
shell scripts

`Synopsis <#toc1>`_
-------------------

**gmt\_init\_tmpdir**

**gmt\_remove\_tmpdir**

**gmt\_clean\_up** [*prefix*\ ]

**gmt\_message** *message*

**gmt\_abort** *message*

**gmt\_nrecords** *file(s)*

**gmt\_nfields** *string*

**gmt\_get\_field** *string*

**gmt\_get\_region** *file(s)* [*options*\ ]

**gmt\_get\_gridregion** *file* [*options*\ ]

**gmt\_get\_map\_width** **-R** **-J**

**gmt\_get\_map\_height** **-R** **-J**

**gmt\_set\_psfile** *file*

**gmt\_set\_framename** *prefix framenumber*

**gmt\_set\_framenext** *framenumber*

`Description <#toc2>`_
----------------------

**gmt\_shell\_functions.sh** provides a set of functions to Bourne
(again) shell scripts in support of **GMT**. The calling shell script
should include the following line, before the functions can be used:

**. gmt\_shell\_functions.sh**

Once included in a shell script, **gmt\_shell\_functions.sh** allows
**GMT** users to do some scripting more easily than otherwise. The
functions made available are:

**gmt\_init\_tmpdir**
    Creates a temporary directory in **/tmp** or (when defined) in the
    directory specified by the environment variable **TMPDIR**. The name
    of the temporary directory is returned as environment variable
    **GMT\_TMPDIR**. This function also causes **GMT** to run in
    ‘isolation mode’, i.e., all temporary files will be created in
    **GMT\_TMPDIR** and the **gmt.conf** file will not be adjusted.
**gmt\_remove\_tmpdir**
    Removes the temporary directory and unsets the **GMT\_TMPDIR**
    environment variable.
**gmt\_cleanup**
    Remove all files and directories in which the current process number
    is part of the file name. If the optional *prefix* is given then we
    also delete all files and directories that begins with the given
    prefix.
**gmt\_message**
    Send a message to standard error.
**gmt\_abort**
    Send a message to standard error and exit the shell.
**gmt\_nrecords**
    Returns the total number of lines in *file(s)*
**gmt\_nfields**
    Returns the number of fields or words in *string*
**gmt\_get\_field**
    Returns the given *field* in a *string*. Must pass *string* between
    double quotes to preserve it as one item.
**gmt\_get\_region**
    Returns the region in the form w/e/s/n based on the data in table
    *file(s)*. Optionally add -IIT(dx)/\ *dy* to round off the answer.
**gmt\_get\_gridregion**
    Returns the region in the form w/e/s/n based on the header of a grid
    *file*. Optionally add -IIT(dx)/\ *dy* to round off the answer.
**gmt\_map\_width**
    Expects the user to give the desired **-R** **-J** settings and
    returns the map width in the current measurement unit.
**gmt\_map\_height**
    Expects the user to give the desired **-R** **-J** settings and
    returns the map height in the current measurement unit.
**gmt\_set\_psfile**
    Create the output *PostScript* file name based on the base name of a
    given file (usually the script name **$0**).
**gmt\_set\_framename**
    Returns a lexically ordered filename stem (i.e., no extension) given
    the file prefix and the current frame number, using a width of 6 for
    the integer including leading zeros. Useful when creating animations
    and lexically sorted filenames are required.
**gmt\_set\_framenext**
    Accepts the current frame integer counter and returns the next
    integer counter.

`Notes <#toc3>`_
----------------

1. These functions only work in the bourne shell (**sh**) and their
derivatives (like **ash**, **bash**, **ksh** and **zsh**). These
functions do not work in the C shell (**csh**) or their derivatives
(like **tcsh**), and cannot be used in DOS batch scripts either.

2. **gmt\_shell\_functions.sh** were first introduced in **GMT** version
4.2.2 and have since been regularly expanded with other practical
scripting short-cuts. If you want to suggest other functions, please do
so by mailing to the GMT mailing list: gmt-help@lists.hawaii.edu.

`See Also <#toc4>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*sh*\ (1) <sh.html>`_ , `*bash*\ (1) <bash.html>`_ ,
`*minmax*\ (1) <minmax.html>`_ , `*grdinfo*\ (1) <grdinfo.html>`_
