.. _Refactoring:

Proposed Refactoring Implementation
===================================

These are the rambling thoughts of Paul Wessel on the C refactoring.  I start with explaining
how the present API came into being and why, what the problems are with the current API, and what the
challenges will be in deciding what the refactoring should accomplish.  This page will be
updated as we debate and reach consensus on various steps.

What is the problem?
--------------------

The GMT API as of GMT 6.x is an unusual beast as far as APIs go.  Let me explain.
When GMT 5 was released we had repackaged all the GMT programs (e.g., pscoast.c, surface.c)
into API modules.  Those source files were still there, but instead of having a main function
required for all C executable programs, they instead defined module functions called *GMT_pscoast*,
*GMT_surface*, etc.  These all had exactly the same argument list, e.g.

::

    int GMT_surface (void *API, int mode, void *args)

where all the arguments necessary for :doc:`/surface` to run were passed via the *args* pointer,
with the *mode* flag indicating what type of arguments were passed.
This module, as all others, would have the following common work flow:

#. Create a linked list of GMT options from the two last arguments
#. Parse GMT common options given to the module
#. Parse module-specific options given to the module
#. Read the input data (if required)
#. Perform the calculation
#. Write output data or *PostScript* (if required)

Since there no longer is a separate **surface** program, for command-line work we rely on the
single program :doc:`/gmt` which does the following:

#. Creates a new GMT session and returns an API pointer to an internal structure
#. Calls the selected module via *GMT_Call_Module* and pass the arguments
#. Destroys the GMT session

The only price users had to pay when moving from GMT 4 to 5 was to get used to running modules as argument to
the **gmt** program, e.g.

::
    
    gmt surface -R0/5/0/6 -I5m my_table.txt -Gmy_grid.grd

and if that was just too hard to get used to we added aliases and functions for shells
as well as symbolic links to gmt in the name of all the modules, so if you compiled GMT
yourself you could pretend to be in "GMT 4-ever" mode and keep running

::

    surface -R0/5/0/6 -I5m my_table.txt -Gmy_grid.grd

However, some modules are built slightly differently as far as i/o goes.  They instead follow this model:

1. Create a linked list of GMT options from the two last arguments
2. Parse any GMT common options given to the module
3. Parse the module-specific options given to the module
4. Loop over input records until EOF

   4.1. Update the main calculation, optionally output new record

5. Finalize calculation, if relevant
6. Write concluding data or *PostScript* (if required)

Let us call this method **B** while the first one we introduced is method **A**.  We will soon return
to the question of why we have two methods in the first place.

Thus, all the work we did to create the API basically resulted in *no change* as far as users go.  So
that begs two questions:

#. Why did we develop an API if it made no difference to users?
#. Why did we implement the above design?

The answer to the first question is this: We wanted to make it possible for a wide range of
users and developers to take advantage of GMT's capabilities from different environments,
such as

#. Custom C, C++, and even FORTRAN programs
#. The GMT modules themselves calling other modules
#. Passing data to and from GMT modules in MATLAB
#. Passing data to and from GMT modules in Python
#. Passing data to and from GMT modules in ???


There had been people who had used GMT from MATLAB and Python before, but
apart from a basic read/write module for grids in MATLAB, any real work would require
writing data to a temp file, using a system call to run a GMT program, and then
reading the result back into MATLAB or Python.  The API was designed to avoid the
system calls and instead call GMT API functions directly with in-memory data, eliminating
the need for temporary files.

However, there was a problem:  The original GMT programs were all stand-along
programs. As discussed above, they would deal with option parsing, the i/o, and the calculations or plotting.
In contrast, MATLAB or Python users may already have data in memory and want
those to be used in the modules *without* temporary files.  We faced a big decision
and not knowing all the ramifications I picked the simplest one that would most
quickly get us to a working system: We simply converted each *main* function of
the C programs to a new callable library function.  That meant all the steps that a module does listed
above now happens inside an API function: parsing of arguments, i/o of data, and
calculations.  An immediate dilemma then was how do you pass data residing in memory into
a module that expects to read from files?  The solution that was selected was
what we now call *virtual files*.  These are special filenames that contain meta-data
about memory locations and are passed as regular filenames.  However, deep in the
under-belly of the GMT i/o layer we know what these filenames mean and instead of
reading a file (e.g., a grid) into a memory structure for a grid we instead
simply pass the memory pointer to the grid we registered as a virtual file directly.  So no
reading, just passing memory (well, I am ignoring some reformatting for special cases here),
and the module is completely unaware of this sleight of hand.
Because of this scheme we were able to convert all ~150 modules
to an API.  The downside came later when we started working on the GMT/MATLAB wrapper
first and later the GMT/Python wrapper (PyGMT).  Unhappy with MATLAB in general,
Joaquim then got excited about the new language Julia, and building on the experience
with GMT/MATLAB he quickly developed a wrapper around the GMT C API to provide a
modern keyword/value interface to GMT from Julia.

What are the refactoring goals?
-------------------------------

The goal of the C refactoring is to produce a second-generation API that
separates the i/o part from the calculation part. Specifically, it would provide
access to a new set of functions that expect input and parameters among the function
arguments and that return output back via other arguments.  For now, we will
focus on modules that do data processing and not plotting, since the latter tend
to be used for adding map layers and there are additional complications here (for instance,
we have no particular interest in receiving an incomplete layer of *PostScript* code).
So, the goal is simple to state.  However, there are complications with this scheme:

1. All modules require a combination of common GMT settings as well as module-specific
   settings. These can range from a few to many.  How complicated do we want the
   function interface to be?  Internally, each module loops over the command-line
   options (step 1 for each model) and assigns a myriad of internal variables that
   are used in the execution of the algorithm. If there is no parsing of any command
   line, how are these set?
2. The input is expected to be passed via function arguments. However, remember
   methods **A** and **B**. In case of **B** we never have the input data in memory
   as they processed record by record from a file.  This was done for several reasons:

   2.1. The algorithm lends itself to this (e.g., sum up values, plot a single point)
   so it was naturally implemented that way -- there was no reason to actually hold
   all the input data in memory at the same time.

   2.2. Some of the processing tools are expected
   to handle very large amounts of data records, beyond what most computers would be
   able to hold in memory.  That, of course, is an ever-moving target, but the concrete
   example we cite is trying to compute spatial averages on a global 15x15 arc-second grid
   for all the multi-beam bathymetry data ever collected, pushing a billion records.
   This may require over 100 Gb of RAM, which in 2021 is still too large. How can we
   handle these special cases?

   2.3. There may not be a simple way to add OpenMP to speed up calculations even if we
   had the data in memory.

Of course, in most cases the amount of data is manageable with method **A** and we
may eventually convert the few model **B** modules to follow model **A**.  Perhaps it
is possible to let a few modules have both models implemented just in case.

How will we pass all those module settings?
-------------------------------------------

Because the GMT API is written in C we are limited to what that language has to offer.
I think there are four ways to pass module options into a new function that can
be called from various environments:

#. Have a long list of possible variables that may be set.  In that case the number
   of variables is *fixed* even if not all are set.  It may look like something like this::

       gmtlib_surface (API, region, increment, registration, tension, input, output);

   where region is an array of *xmin, xmax, ymin, ymax* and increment holds the *xinc* and
   *yinc* increments, etc., etc.  However, :doc:`/surface` has many more settings that a user may
   wish to access, so the number of arguments will quickly grow.  Having a function with
   10-20 arguments is not friendly. Furthermore, as soon as it is released, the developers
   decide to add yet another argument and now we must break the interface in the next release.
#. Another scheme relies on collecting all the possible options into a structure and then
   just pass the pointer to that structure to the function.  That way, we can still add new
   settings as new members of the structure without breaking the function interface.  We
   do something like this when GMT communicates with GDAL.
#. The C language allows for a function with a *variable-length* argument list.  This makes it possible to implement a
   rudimentary keyword, value system where each string keyword is followed by a void pointer
   to the argument.  Internal parsing will use the keyword to know what its argument should
   be and obtain the value.  This avoids the breaking of the interface since optional arguments
   do not have to be listed.  It might look something like this::

       gmtlib_surface (API, input, output, "region", wesn, "increment", incs, NULL);

   where we have changed the location of the input and output arguments so that the variable
   list of keyword-value pairs can end with the required NULL pointer.
#. Finally, we could imagine some combination of the single structure pointer and the
   NULL-terminated list of keyword/argument pairs.  This is the one I am leaning towards.


But first, in considering these four schemes I think we can exclude the first: it is simply unwieldy
given the many options each module may offer.  The second scheme (structure) seems at first
promising: In fact, all modules already have such a structure that we use when we parse the
options internally.  For instance, the surface module has *struct SURFACE_CTRL* with sub-structures
for each of the module-specific settings (*T* for tension, *N* for iteration count, *Z* for over-relaxation, etc.).
While currently a private structure only used internally by the module, one can imagine making it an
external structure so that developers could directly create a copy and fill in the values for
the sub-structures (options) they wish. Alas, this is not a simple procedure.  The regular
parser functions (one which there is one for each module) often do considerable work in translating a user
text-string option into internal parameters.  It is often not possible to simply enter the
value of the resulting parameter directly because intermediate steps are required that
calculate the final parameter.  Hence, this scheme seems too low-level to be a flexible
alternative to the current high-level (but limited) options strings, although we should look
at this in detail before making that final determination.  With that caveat, this leaves us with
the last two schemes.  I think either would allow workable solutions.  However, to ensure we
are able to document the functions, maintain the current command-line option parsers, and
allow for the long-option expansion discussed previously, I suggest the best solution may
be a combination: We create a keyword-argument basic structure and wrap an array of these
under a new GMT dictionary structure. It is this single pointer that we pass in for the
settings, but it is not filled out like the low-level structure above but populated from
a keyword-argument interface.

Here is a sequence of commands that demonstrate how this might work.  I am using C
syntax here but I anticipate simple wrappers around these if other languages need some
variations on the theme::

    dict = gmt_create_dictionary (API); /* Create a new and empty GMT dictionary */
    gmt_put_option (API, dict, "region", GMT_DOUBLE, 4, wesn);   /* Pass wesn array */
    gmt_put_modifier (API, dict, "region", "selection", GMT_CHAR, 0, "diagonal");   /* Pass modifier +r or +selection=diagonal */
    gmt_put_option (API, dict, "increment", GMT_DOUBLE, 2, incs);   /* Pass incs array */
    gmt_put_option (API, dict, "registration", GMT_INT, 1, GMT_GRID_PIXEL_REG);
    gmt_put_option (API, dict, "tension", GMT_DOUBLE, 1, tension);

We can now call the module::

    gmtlib_surface (API, dict, input, output);

Internally, we simply use the dictionary to create a linked list of GMT options and let the
standard module parser translate the dictionary settings to the internal structure parameters.
By using the long-options format for options and modifiers we simply use that as the documentation
for both the command-line options/modifiers and the dictionary settings, and we can then easily
produce one from the other. In other words, it builds on existing parsing schemes.

How will we pass input and output?
----------------------------------

But what about the data that need to be passed in and out of a module? No longer relying
on virtual files, we need to pass memory locations.  There are some complications here too:

#. Some environments may wish to pass a matrix for table input while others may wish
   to pass a set of column vectors. While many environments are able to combine column
   vectors into a matrix easily, others (e.g., C) cannot.
#. Some environments use column-order while others use row-order; this affects how matrices
   can be processed.
#. A few modules may still wish to give a filename instead of reading the data in first,
   as explained during our discussion of model **B**.
#. Many use cases want to receive the data back to the calling environment but some may
   wish to write to a file directly (again, because it may just be too big).
#. While passing in a table of data is not complicated, once we bring in grids there is
   the complication of how a grid is represented in the environment.  Perhaps it is a matrix.
   Then, there has to be additional variables to inform on region and grid spacing, for instance.
   Is the matrix a set of rows or a set of columns?  What type is the grid (float, integer, char)?
   How much of this variability should be support at this API level?

Taking a lesson from how we handled settings, the simplest scheme for input and output is
to use a structure to pass the various items.  Unlike the complexity of the myriad of
module-specific options and sub-structures, for input and output we may be able to use a common
IO structure such as this mock-up::

    struct GMT_IO {
        unsigned int mode;
        unsigned int type;
        void *data;
    };

where the *type* integer controls *what* type of data is stored via the *data* pointer,
and the *mode* integer controls *how* it is stored.  I suggest a separate structure is
used for input and output since for some modules we may need to pass NULL for one or
both of them to maintain the same interface with four arguments across all modules.
The loading of the IO structure can be done via functions as well:

::

    gmt_put_input (API, struct GMT_IO item, unsigned int type, unsigned int mode);

with an example of passing a matrix to surface via a struct GMT_IO pointer looking like this::

    gmt_define_io (API, input, GMT_DOUBLE, GMT_IS_MATRIX);
