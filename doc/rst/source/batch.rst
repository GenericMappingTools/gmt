.. index:: ! batch
.. include:: module_core_purpose.rst_

*****
batch
*****

|batch_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt batch** *mainscript*
|-N|\ *prefix*
|-T|\ *njobs*\|\ *min*/*max*/*inc*\ [**+n**]\|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**\ [*str*]]
[ |-I|\ *includefile* ]
[ |-M|\ [*job*] ]
[ |-Q|\ [**s**] ]
[ **-Sb**\ *preflight* ]
[ **-Sf**\ *postflight* ]
[ |SYN_OPT-V| ]
[ |-W|\ [*workdir*] ]
[ |-Z| ]
[ |SYN_OPT-x| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

The **batch** module can generate GMT processing jobs using a single master script
that is repeated for all jobs, with some variation using specific job variables.  The
module simplifies (and hides) most of the steps normally needed to set up a full-blown
processing sequence.  Instead, the user can focus on composing the main processing script and let the
parallel execution of jobs be automatic.  We can set up required data sets and do one-time calculations
via an optional *preflight* script.  After completion we can optionally assemble the data output
and make summary plots or similar in the *postflight* script.


Required Arguments
------------------

*mainscript*
    Name of a stand-alone GMT modern mode processing script that makes the parameter-dependent calculations.  The
    script may access job variables, such as job number and others defined below, and may be
    written using the Bourne shell (.sh), the Bourne again shell (.bash), the csh (.csh)
    or DOS batch language (.bat).  The script language is inferred from the file extension
    and we build hidden batch scripts using the same language.  Parameters that can be accessed
    are discussed below.

.. _-N:

**-N**\ *prefix*
    Determines the prefix of the batch file products and the final sub-directory with all job products.

.. _-T:

**-T**\ *njobs*\|\ *min*/*max*/*inc*\ [**+n**]\|\ *timefile*\ [**+p**\ *width*]\ [**+s**\ *first*]\ [**+w**\ [*str*]]
    Either specify how many jobs to make, create a one-column data set width values from
    *min* to *max* every *inc* (append **+n** if *inc* is number of jobs instead), or supply a file with
    a set of parameters, one record (i.e., row) per job.  The values in the columns will be available to the
    *mainscript* as named variables **BATCH_COL0**, **BATCH_COL1**, etc., while any trailing text
    can be accessed via the variable **BATCH_TEXT**.  Append **+w** to split the trailing
    string into individual *words* that can be accessed via variables **BATCH_WORD0**, **BATCH_WORD1**,
    etc. By default we use any white-space to separate words.  Append *str* to select another character(s)
    as the valid separator(s). The number of records equals the number of jobs. Note that the *preflight* script is allowed to
    create *timefile*, hence we check for its existence both before *and* after the *preflight* script has
    completed.  Normally, the job numbering starts at 0; you can change this by appending a different starting
    job number via **+s**\ *first*.  **Note**: All jobs are still included; this modifier only affects
    the numbering of the given jobs.  Finally, **+p** can be used to set the tag *width* of the format
    used in naming jobs.  For instance, name_000010.grd has a tag width of 6.  By default, this is
    automatically set but if you are splitting large jobs across several computers (via **+s**) then you
    must use the same tag width for all names.


Optional Arguments
------------------

.. _-I:

**-I**\ *includefile*
    Insert the contents of *includefile* into the batch_init.sh script that is accessed by all batch scripts.
    This mechanism is used to add information (typically constant variable assignments) that the *mainscript*
    and any optional **-S** scripts can rely on.

.. _-M:

**-M**\ [*job*]
    Instead of making and launching the full processing sequence, select a single master job [0] for testing.
    The master job will be run and its product(s) are placed in the top directory. While any *preflight* script
    will be run prior to the master job, the *postflight* script will not be executed (but it will be created).

.. _-Q:

**-Q**\ [**s**]
    Debugging: Leave all files and directories we create behind for inspection.  Alternatively, append **s** to
    only build the batch scripts but *not* perform any executions.  One exception involves the optional
    *preflight* script derived from **-Sb** which is always executed since it may produce data needed when
    building the main batch (or master) scripts.

.. _-Sb:

**-Sb**\ *preflight*
    The optional GMT modern mode *preflight* (written in the same scripting language as *mainscript*) can be
    used to download or copy data files or create files (such as *timefile*) that will be needed by *mainscript*.
    It is always run **b**\ efore the main sequence of batch scripts.

.. _-Sf:

**-Sf**\ *postflight*
    The optional *postflight* (written in the same scripting language as *mainscript*) can be
    used to perform final processing steps **f**\ ollowing the completion of all the individual jobs, such as
    assembling all the products into a single larger file.  The script may also make one or more illustrations
    using the products or stacked data after the main processing is completed. It does not have to be a GMT
    script.

.. _batch-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [*workdir*]
    By default, all temporary files and job products are created in the subdirectory *prefix* set via **-N**.
    You can override that selection by giving another *workdir* as a relative or full directory path. If no
    path is given then we create a working directory in the system temp folder named *prefix*.  The main benefit
    of a working directory is to avoid endless syncing by agents like DropBox or TimeMachine, or to avoid
    problems related to low space in the main directory.  The product files will still be placed in the *prefix*
    directory.  The *workdir* is removed unless **-Q** is specified for debugging.

.. _-Z:

**-Z**
    Erase the *mainscript* and all input scripts given via **-I** and **-S** upon completion.  Not compatible
    with **-Q**.

.. _-cores:

**-x**\ [[-]\ *n*]
    Limit the number of cores to use when loading up the cores.
    By default we try to use all available cores.  Append *n* to only use *n* cores
    (if too large it will be truncated to the maximum cores available).  Finally,
    give a negative *n* to select (all - *n*) cores (or at least 1 if *n* equals or exceeds all).
    The parallel processing does not depend on OpenMP; new jobs are launched when the previous ones
    complete.

.. include:: explain_help.rst_

Parameters
----------

Several parameters are automatically assigned and can be used when composing the *mainscript* and the
optional *preflight* and *postflight* scripts. There are two sets of parameters: Those that are constants
and those that change with the job number.  The constants are accessible by all the scripts:
**BATCH_PREFIX**\ : The common prefix of the batch jobs (it is set with **-N**). **BATCH_NJOBS**\ : The
total number of jobs (given or inferred from **-T**). Also, if **-I** was used then any static parameters
listed therein will be available to all the scripts as well. In addition, the *mainscript* also has access
to parameters that vary with the job counter: **BATCH_JOB**\ : The current job number (an integer, e.g., 136),
**BATCH_TAG**\ : The formatted job number given the precision (a string, e.g., 000136), and **BATCH_NAME**\ :
The name prefix unique to the current job (i.e., *prefix*\ _\ **BATCH_TAG**), Furthermore, if a *timefile*
was given then variables **BATCH_COL0**\ , **BATCH_COL1**\ , etc. are also set, yielding one variable per
column in *timefile*.  If *timefile* has trailing text then that text can be accessed via the variable
**BATCH_TEXT**, and if word-splitting was explicitly requested by **+w** modifier to **-T** then the trailing
text is also split into individual word parameters **BATCH_WORD0**\ , **BATCH_WORD1**\ , etc. **Note**: Any
product(s) made by the processing scripts should be named using **BATCH_NAME** as their name prefix as these
will be automatically moved up to the starting directory upon completion.

Data Files
----------

The batch scripts will be able to find any files present in the starting directory when **batch** was initiated,
as well as any new files produced by *mainscript* or the optional scripts set via **-S**.
No path specification is needed to access these files.  Other files may
require full paths unless their directories were already included in the :term:`DIR_DATA` setting.

Constructing the Main Script
----------------------------

A batch sequence is not very interesting if nothing changes between calls.  For the process to change you need
to have your *mainscript* either access a *different* data set as the job number changes, or you need to access
only a varying *subset* of a data set, or the processing parameters need to change, or all of the above.  There
are several strategies you can use to accomplish these effects:

#. Your *timefile* passed to **-T** may list names of specific data files and you simply have your *mainscript*
   use the relevant **BATCH_TEXT** or **BATCH_WORD?** to access the job-specific file name.
#. You have a 3-D grid (or a stack of 2-D grids) and you want to interpolate along the axis perpendicular to the
   2-D slices (e.g., time, or it could be depth).  In this situation you will use the module :doc:`grdinterpolate`
   to have the *mainscript* obtain a slice for the correct time (this may be an interpolation between two different
   times or depths) and process this temporary grid file.
#. You may be creating data on the fly using :doc:`gmtmath` or :doc:`grdmath`, or perhaps processing data slightly
   differently per job (using parameters in the *timefile*) and computing these or the changes between jobs.
#. Use your imagination to pass whatever arguments are needed via *timefile*.


Technical Details
-----------------

The **batch** module creates several hidden script files that are used in the generation of the products
(here we have left the script file extension off since it depends on the scripting language used): *batch_init*
(initializes variables related to the overall batch job and includes the contents of the optional *includefile*),
*batch_preflight* (optional since it derives from **-Sb** and computes or prepares needed data files), *batch_postflight*
(optional since it derives from **-Sf** and processes files once all the batch job complete), *batch_job*
(accepts a job counter argument and processes data for those parameters), and *batch_cleanup* (removes temporary
files at the end of the process). For each job, there is a separate *batch_params_######* script that provides
job-specific variables (e.g., job number and anything given via **-T**).  The *preflight* and *postflight* scripts
have access to the information in *batch_init*, while the *batch_job* script in addition has access to the job-specific
parameter file.  Using the **-Q** option will just produce these scripts which you can then examine.
**Note**: The *mainscript* is duplicated per job and many of these are run simultaneously on all available cores.
Multi-treaded GMT modules will therefore be limited to a single core per call.  Because we do not know how
many products each batch job makes, we ensure each job creates a unique file when it is finished.  Checking for
these special (and empty) files is how **batch** learns that a particular job has completed and it is time to
launch another one.


Hints for Batch Makers
----------------------

Composing batch jobs is relatively simple, but you have to think in terms of *variables*. Examine the examples
we describe.  Then, start by making a single script (i.e., your *mainscript*) and identify which
things should change with time (i.e., with the job number).  Create variables for these values. If they
are among the listed parameters that **batch** creates automatically then use those names.  Unless you only
require the job number you will need to make a file that you can pass via **-T**.  This file should
then have all the values you need, per job (i.e., per row), with values across all the columns you need.
If you need to assign various *fixed* variables that do not change with time, then your *mainscript*
will look shorter and cleaner if you offload those assignments to a separate *includefile* (via **-I**).
To test your *mainscript*, start by using options **-Q -M** to ensure that your master job results are correct.
The **-M** option simply runs one job of your batch sequence (you can select which one via the **-M**
arguments [0]).  Fix any issues with your use of variables and options until this works.  You can then try
to remove **-Q**. We recommend you make a very short (i.e., via **-T**) and small batch sequence so you don't
have to wait very long to see the result.  Once things are working you can beef up number of jobs.


Examples
--------

We extract a subset of bathymetry for the Gulf of Guinea from the 2x2 arc minute resolution Earth DEM and compute
Gaussian filtered high-pass grids using filter widths ranging from 10 to 200 km in steps of 10 km. When the grids
are all completed we determine the standard deviation in the result.  To replicate our setup, try::

    cat << EOF > pre.sh
    gmt begin
        gmt math -o0 -T10/200/10 T = widths.txt
        gmt grdcut -R-10/20/-10/20 @earth_relief_02m -Gdata.grd
    gmt end
    EOF
    cat << EOF > main.sh
    gmt begin
        gmt grdfilter data.grd -Fg\${BATCH_COL0}+h -G\${BATCH_NAME}.grd -D2
    gmt end
    EOF
    cat << EOF > post.sh
    gmt begin \${BATCH_PREFIX} pdf
        gmt grdmath \${BATCH_PREFIX}_*.grd -S STD = \${BATCH_PREFIX}_std.grd
        gmt grdimage \${BATCH_PREFIX}_std.grd -B -B+t"STD of Gaussians residuals" -Chot
        gmt coast -Wthin,white
    gmt end show
    EOF
    gmt batch main.sh -Sbpre.sh -Sfpost.sh -Twidths.txt -Nfilter -V -Z
    
Of course, the syntax of how variables are used vary according to the scripting language. Here, we actually
build the pre.sh, main.sh, and post.sh scripts on the fly, hence we need to escape any variables (since they
start with a dollar sign that we need to be written verbatim). At the end of the execution we find 20 grids
(e.g., such as filter_07.grd), as well as the filter_std.grd file obtained by stacking all the individual
scripts and computing a standard deviation. The information needed to do all of this is hidden from the user;
the actual batch scripts that we execute are derived from the user-provided main.sh script and **batch**
supplies the extra machinery. The **batch** module automatically manages the parallel execution loop over all
jobs using all available cores and launches new jobs as old ones complete.

As another example, we get a list of all European countries and make a simple coast plot of each of them,
placing their name in the title and the 2-character ISO code in the upper left corner, then in postflight
we combine all the individual PDFs into a single PDF file and delete the individual files::

    cat << EOF > pre.sh
    gmt begin
        gmt coast -E=EU+l > countries.txt
    gmt end
    EOF
    cat << EOF > main.sh
    gmt begin \${BATCH_NAME} pdf
        gmt coast -R\${BATCH_WORD0}+r2 -JQ10c -Glightgray -Slightblue -B -B+t"\${BATCH_WORD1}" -E\${BATCH_WORD0}+gred+p0.5p
        echo \${BATCH_WORD0} | gmt text -F+f16p+jTL+cTL -Gwhite -W1p
    gmt end
    EOF
    cat << EOF > post.sh
    gs -dQUIET -dNOPAUSE -sDEVICE=pdfwrite -sOUTPUTFILE=\${BATCH_PREFIX}.pdf -dBATCH \${BATCH_PREFIX}_*.pdf
    rm -f \${BATCH_PREFIX}_*.pdf
    EOF
    gmt batch main.sh -Sbpre.sh -Sfpost.sh -Tcountries.txt+w"\t" -Ncountries -V -W -Zs

Here, the postflight script is not even a GMT script; it simply runs gs (Ghostscript) and deletes what we don't want to keep.

See Also
--------

:doc:`gmt`,
:doc:`gmtmath`,
:doc:`grdinterpolate`,
:doc:`grdmath`
