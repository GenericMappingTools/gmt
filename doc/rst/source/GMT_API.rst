*******
GMT API
*******

roman GMT\ :sub:`C`\ over

headings

headings

Introduction
============

arabic headings

Preamble
--------

[h] |image| GMT 4 programs contain all the high-level functionality.

Prior to version 5, the bulk of  functionality was coded directly in the
standard  C program modules (e.g., surface.c, psxy.c, grdimage.c). The
 library only offered access to lower-level functions from which those
high-level  programs were built. The standard  programs have been very
successful, with tens of thousands of users world-wide. However, the
design of the main programs prevented developers from leveraging
 functionality from within other programming environments since access
to  tools could only be achieved via system calls. Consequently, all
data i/o had to be done via temporary files. The design also prevented
the  developers themselves from taking advantage of these modules
directly. For instance, the tool pslegend needed to make extensive use
of system calls to psxy and pstext in order to plot the lines, symbols
and text that make up a map legend.

[h] |image1| GMT 4 programs contain all the high-level functionality.

Starting with  version 5, all standard  programs are split into short
driver program (the “new”   programs) and a function “module”. The
drivers simply call the corresponding  modules; it is these modules that
do most of the work. These new functions have been placed in a new
 high-level API library and can be called from a variety of environments
(C/C++, Fortran, Python, Matlab, Visual Basic, etc.) [1]_. For example,
the main program blockmean.c has been reconfigured as a high-level
function GMT\_blockmean(), which does the actual spatial averaging and
passes the result back to the calling program. The previous behavior of
blockmean.c is replicated by a short driver program that simply collects
user arguments and then calls GMT\_blockmean(). Indeed, the driver
programs for all the standard  programs are so short and simple that the
makefile generates them on-the-fly when it compiles and links them with
the  library into executables. Thus, blockmean.c and others do no longer
exist.

In order for this interface to be as flexible as possible we have
generalized the notion of input and output. Now, data that already
reside in an application’s memory may serve as input to a  function.
Other sources of input may be file pointers and file descriptors (as
well as the already-supported mechanism for passing file names). For
standard data table i/o, the  API takes care of the task of assembling
any combination of files, pointers, and memory locations into a single
virtual data set from which the function may read (a) all records at
once into memory, or (b) read one record at a time. Likewise,  functions
may write their output to a virtual destination, which might be a memory
location in the user’s application, a file pointer or descriptor, or an
output file. The  functions are unaware of these details and simply read
from a “source” and write to a “destination”.

Here, we document the new functions in the  API library for application
developers who wish to call these functions from their own custom
programs. At this point, only the new high-level  API is fully
documented and intended for public use. The structure and documentation
of the under-lying lower-level  library is not finalized Developers
using these functions may risk disruption to their programs due to
changes we may make in the library in support of the  API.

Definitions
-----------

For the purpose of this documentation a few definitions are needed:

#. “Standard  program” refers to one of the traditional stand-alone
   command-line executables known to all  users, e.g., blockmean, psxy,
   grdimage, etc. Prior to version 5, these were the only  executables
   available.

#. “ module” refers to the function in the  API library that is
   responsible for all the action taken by the corresponding  program.
   All such modules are given the same name as the corresponding program
   but carry the prefix ``GMT_``, e.g., ``GMT_blockmean``.

#. “ application” refers to a new application written by developers and
   may call one or more  functions to create a new -compatible
   executable.

#. In the API description that follows we will use the type ``long`` to
   mean a 4-byte (for 32-bit systems) or 8-byte (for 64-bit systems)
   integer. Since different operating systems have their own way of
   defining 8-byte integers it is recommended that developers use the
   type ``GMT_LONG`` for this purpose; it is guaranteed to yield the
   correct type that the  functions expect.

In version 5, the standard  programs are themselves specific but overly
simple examples of  applications that only call the single  function
they are associated with. However, some exceptions such as pslegend and
gmtconvert call several modules.

Recognized resources
--------------------

The  API knows how to read and write five types of data common to
 operations: CPT palette tables, data tables (ASCII or binary), text
tables,  grids and images (reading only). In addition, we have two data
types to facilitate the passing of simple user arrays (one or more data
columns of any data type, e.g., double, char) and 2-D user matrices (of
any data type and column/row organization). There are many attributes
for each of these two entities and therefore we use a top-level
structure for each to keep them all in one container. These containers
are given or returned by the  API functions using pointers (``void *``).
Below we discuss these containers in some detail; we will later present
how they are used when importing or exporting them to or from files,
memory locations, or streams. The first five are the standard  objects,
while the latter two are the special user data containers.

CPT palette tables
~~~~~~~~~~~~~~~~~~

The color palette table files, or just CPT tables, contain colors and
patterns used for plotting data such as surfaces (i.e.,  grids) or
symbols, lines and polygons (i.e.,  tables).  programs will generally
read in a CPT palette table, make it the current palette, do the
plotting, and destroy the table when done. The information is referred
to via a pointer to ``struct GMT_PALETTE``. Thus, the arguments to  API
functions that handle palettes expect this type of variable.

Data tables
~~~~~~~~~~~

Much data processed in  come in the form of ASCII or binary data tables.
These may have any number of header records (ASCII only) and perhaps
segment headers.  programs will read one or many such tables when
importing data. However, to avoid memory duplication some programs may
prefer to read records one at the time. The  API has functions that let
you read record-by-record by presenting a virtual data set that combines
all the data tables specified as input. This simplifies record
processing considerably. A ``struct GMT_DATASET`` may contain any number
of tables, each with any number of segments, and each segment with any
number of records. Thus, the arguments to  API functions that handle
such data sets expect this type of variable. All segments are expected
to have the same number of columns.

Text tables
~~~~~~~~~~~

Some data needed by  are simply free-form ASCII text tables. These are
handled similarly to data tables. E.g., they may have any number of
header records and even segment headers, and  programs can read one or
more tables or get text records one at the time. A
``struct GMT_TEXTSET`` may contain any number of tables, each with any
number of segments, and each segment with any number of records. Thus,
the arguments to  API functions that handle such data sets expect this
type of variable.

GMT grids
~~~~~~~~~

 grids are used to represent equidistant and organized 2-D surfaces.
These can be plotted as contour maps, color images, or as perspective
surfaces. Because the native  grid is simply a 1-D float array with all
the metadata kept in a separate header, we pass this information via a
``struct GMT_GRID``, which is a container that holds both items. Thus,
the arguments to  API functions that handle such  grids expect this type
of variable.

GMT images
~~~~~~~~~~

 images are used to represent bit-mapped images obtained via the GDAL
bridge. These can be reprojected internally, such as when used in
grdimage. Since images and grids share the concept of a header, we use
the same header structure for grids as for images; however, some
additional metadata attributes are also needed. Finally, the image
itself may be of any data type. Both image and header information are
passed via a ``struct GMT_IMAGE``, which is a container that holds both
items. Thus, the arguments to  API functions that handle such  images
expect this type of variable. Unlike the other objects, images can only
be read and not written.

User data columns
~~~~~~~~~~~~~~~~~

[h]

ll

2l\ ``union GMT_UNIVECTOR {``
`` unsigned char *uc1;`` & /\* *Pointer for unsigned char array* \*/
`` char *sc1;`` & /\* *Pointer for signed char array* \*/
`` unsigned short *ui2;`` & /\* *Pointer for unsigned short array* \*/
`` short *si2;`` & /\* *Pointer for signed short array* \*/
`` unsigned int *ui4;`` & /\* *Pointer for unsigned int array* \*/
`` int *si4;`` & /\* *Pointer for signed int array* \*/
`` unsigned long *ui8;`` & /\* *Pointer for unsigned long array* \*/
`` long *si8;`` & /\* *Pointer for signed long array* \*/
`` float *f4;`` & /\* *Pointer for float array* \*/
`` double *f8;`` & /\* *Pointer for double array* \*/
``};`` &

Definition of the GMT\_UNIVECTOR union that hold a pointer to any array
type. [tbl:univector]

[h]

ll

2l\ ``struct GMT_VECTOR {``
`` long id;`` & /\* *An identification number* \*/
`` long n_rows;`` & /\* *Number of rows in each vector* \*/
`` long n_columns;`` & /\* *Number of vectors* \*/
`` long alloc_mode;`` & /\* *Determines if we may free the vectors or
not* \*/
`` long *type;`` & /\* *Array with data type for each vector* \*/
`` union GMT_UNIVECTOR *data;`` & /\* *Array with unions for each
column* \*/
``};`` &

Definition of the GMT\_VECTOR structure used to pass user data columns.
[tbl:vector]

Programs that may wish to call  modules may have input data in their own
particular structures. For instance, the user’s program may have three
column arrays of type float and wishes to use these as the input source
to the ``GMT_surface`` module, which normally expects a
``struct GMT_DATASET`` via file or reference. Simply create a
``struct GMT_VECTOR`` (see section [sec:create]) and assign the union
array pointers (see Table [tbl:univector]) to your data columns and
provide the required information on length and data types (see
Table [tbl:vector]). By letting the  module know you are passing a data
set via a ``struct GMT_VECTOR`` it will know how to read the data
properly.

User data matrices
~~~~~~~~~~~~~~~~~~

[h]

ll

2l\ ``struct GMT_MATRIX {``
`` long id;`` & /\* *An identification number* \*/
`` long n_rows;`` & /\* *Number of rows in the matrix* \*/
`` long n_columns;`` & /\* *Number of columns in the matrix* \*/
`` long n_layers;`` & /\* *Number of layers in a 3-D matrix* \*/
`` long registration;`` & /\* *0 for gridline and 1 for pixel
registration* \*/
`` long shape;`` & /\* *0 = C (rows) and 1 = Fortran (cols)* \*/
`` long dim;`` & /\* *Length of dimension for row (C) or column
(Fortran)* \*/
`` long alloc_mode;`` & /\* *Determines if we may free the vectors or
not* \*/
`` long type;`` & /\* *The matrix data type* \*/
`` double limit[6];`` & /\* *The min and max limits on x-, y-, and
z-ranges* \*/
`` union GMT_UNIVECTOR data;`` & /\* *Union with pointers a data matrix
of any type* \*/
``};`` &

Definition of the GMT\_MATRIX structure used to pass a user data matrix.
[tbl:matrix]

Likewise, a programs may have an integer 2-D matrix in memory and wish
to use that as the input grid to the ``GMT_grdfilter`` module, which
normally expects a ``struct GMT_GRID`` via file or reference. As for
user vectors, we create a ``struct GMT_MATRIX`` (see
section [sec:create]), assign the appropriate union pointer to your data
matrix and provide information on dimensions and data type (see
Table [tbl:matrix]). Letting the  module know you are passing a grid via
a ``struct GMT_MATRIX`` it will know how to read the matrix properly.

Overview of the GMT C Application Program Interface
===================================================

Users who wish to create their own  applications based on the API must
make sure their program goes through the steps below; details for each
step will be revealed in the sections to follow. We have kept the API
simple: In addition to the  modules, there are only 17 public functions
to become familiar with. All functions sets the variable ``API->error``
to the appropriate error code (when things go wrong); otherwise it is
set to GMT\_OK (0). The layout here assumes you wish to use data in
memory as input sources; if the data are simply command-line files then
things simplify considerably.

#. Initialize a new  session by calling ``GMT_Create_Session``, which
   allocates a  API control structure and returns a pointer to it. This
   pointer must be used as first argument to all subsequent  API
   function calls within the same session.

#. For each intended call to a  function, several steps are involved:

   #. Register the input sources and register the output destination
      using ``GMT_Register_IO``, unless you know you are working with a
      single file or standard input/output. The resources will typically
      be files, memory locations, already-opened file handles, and even
      process streams.

   #. Each resource registration will generate a unique ID number. For
      memory resources, these numbers are then converted to unique
      filenames of the form “@GMTAPI@-######” that are used with
       modules. When  i/o library functions encounter such filenames
      they extract the ID and make a connection to the resource
      registered under that ID. Any number of table data or text sources
      will be combined into a single virtual source for  functions to
      operate on. In contrast, CPT, Grid, and image resources are
      operated on individually.

   #. Enable data import once all registrations are complete.

   #. Read into memory all data that will be passed to  modules via
      pointers. You may choose to read everything into memory at once or
      process the data record-by-record (tables only).

   #. Prepare the program options required and call the  module you wish
      to use.

   #. Process the results that were returned to memory via pointers
      rather than written to files.

   #. Explicitly destroy the resources allocated by  modules to hold the
      results, or let the garbage collector do this automatically at the
      end of the module and at the end of the session.

#. Repeat steps a–f as many times as your application requires. All API
   functions return a status code which is GMTAPI\_OK (0) if all is
   well. For non-zero return values, use ``GMT_Report_Error`` to
   generate an error message.

#. We terminate the GMT session by calling ``GMT_Destroy_Session``.

Advanced programs may be calling more than one  session and thus run
several sessions, perhaps concurrently as different threads on
multi-core machines. We will now discuss these steps in more detail.

Initialize a new GMT session
----------------------------

Most applications will need to initialize only a single  session. This
is true of all the standard  programs since they only call one  module
and then exit. Most user-developed  applications are likely to only
initialize one session even though they may call many  modules. However,
the  API supports any number of simultaneous sessions should the
programmer wish to take advantage of it. This might be useful when you
have access to several CPUs and want to spread the computing load [2]_.
In the following discussion we will simplify our treatment to the use of
a single session only.

The ``GMT_Create_Session`` is used to initiate the new session. The full
function prototype is

::

    struct GMTAPI_CTRL * GMT_Create_Session (char *tag, long mode)

and you will typically call it thus:

::

    GMT_LONG mode = GMTAPI_GMT;
    struct GMTAPI_CTRL *API = NULL;
    API = GMT_Create_Session ("Session name", mode);

where ``API`` is a pointer to the allocated  API control structure. You
will need to pass this pointer to *all* subsequent  API functions. The
key task of this initialization is to set up the  machinery and its
internal variables used for map projections, plotting, etc. The
initialization also allocates space for internal structures used to
register resources. If you expect to call modules that also require the
PSL library, then set ``mode`` to GMTAPI\_GMTPSL (1); else simply pass
GMTAPI\_GMT (0). Should something go wrong then ``API`` will be returned
as ``NULL``.

Register input or output resources
----------------------------------

When using the standard  programs, you specify input files on the
command line or via special program options (e.g., I\ *intensity.nc*).
The output of the programs are either written to standard output (which
you redirect to files or pipe to other programs) or to files specified
by specific program options (e.g., G\ *output.nc*). However, the  API
allows you to also specify input (and output) to come from (or go to)
open file handles or program memory locations. We will examine this more
closely below. Registering a resource is a required step before
attempting to import or export data other that via file options and
standard input/output.

Resource registration
~~~~~~~~~~~~~~~~~~~~~

The basic registration machinery involves a direct or indirect call to

::

    long GMT_Register_IO (struct GMTAPI_CTRL *API, long family, long method, \
       long geometry, long direction, void *ptr, double wesn[])

where ``family`` specifies what kind of resource is to be registered
(see Table [tbl:family] for list of all families), ``method`` specifies
how we expect to access this resource (see Table [tbl:methods] for
recognized methods, as well as modifiers you can add; these are listed
in Table [tbl:via]), ``geometry`` specifies the geometry of the data
(see Table [tbl:geometry] for recognized geometries), ``ptr`` is the
address of the pointer to the named input resource. If ``direction`` is
GMT\_OUT and the ``method`` is not related to a file (filename, stream,
or handle), then ``ptr`` must be NULL. After the  module has written the
data you can use ``GMT_Retrieve_Data`` to assign a pointer to the memory
location where the output container was allocated. For grid (and image)
resources you may request to obtain a subset via the ``wesn`` array (see
Table [tbl:wesn] for information); otherwise, pass NULL to obtain the
entire grid (or image). The ``direction`` indicates input or output and
is either GMT\_IN (0) or GMT\_OUT (1). Finally, the function returns a
unique resource ID, or GMTAPI\_NOTSET (-1) if there was an error (with
error code returned via ``API->error``).

Object ID encoding
~~~~~~~~~~~~~~~~~~

If registered resources are to be given as program input or output
arguments you will need to pass them via a text string that represents a
special file name. The proper filename formatting is guaranteed by using
the function

::

    long GMT_Encode_ID (struct GMTAPI_CTRL *API, char *filename, long ID)

which accepts the unique ``ID`` and writes the ``filename`` that you can
use as argument to a program option. ``filename`` must have enough space
to hold 16 bytes. The function returns TRUE (1) if there is an error
(which is passed back with ``API->error``), otherwise it returns FALSE
(0).

[h]

+--------------------+--------------------------------+
| 1\|c\|\ *family*   | 1c\|\ *source points to*       |
+====================+================================+
| GMT\_IS\_DATASET   | A [multi-segment] table file   |
+--------------------+--------------------------------+
| GMT\_IS\_TEXTSET   | A [multi-segment] text file    |
+--------------------+--------------------------------+
| GMT\_IS\_GMTGRID   | A  grid file                   |
+--------------------+--------------------------------+
| GMT\_IS\_CPT       | A CPT file                     |
+--------------------+--------------------------------+

Integer constants defined for use when specifying input or output data
families. [tbl:family]

[h]

+---------------------+-----------------------------------------------------------+
| 1\|c\|\ *method*    | 1c\|\ *how to read/write data*                            |
+=====================+===========================================================+
| GMT\_IS\_FILE       | Pointer to name of a file                                 |
+---------------------+-----------------------------------------------------------+
| GMT\_IS\_STREAM     | Pointer to open file (or process)                         |
+---------------------+-----------------------------------------------------------+
| GMT\_IS\_FDESC      | Pointer to integer file descriptor                        |
+---------------------+-----------------------------------------------------------+
| GMT\_IS\_COPY       | Pointer to memory to *copy* data from                     |
+---------------------+-----------------------------------------------------------+
| GMT\_IS\_REF        | Pointer to memory to *reference* data from (realloc OK)   |
+---------------------+-----------------------------------------------------------+
| GMT\_IS\_READONLY   | Pointer to memory to *read* data from                     |
+---------------------+-----------------------------------------------------------+

Integer constants defined for use when specifying input or output
methods. [tbl:methods]

[h]

+----------------------+---------------------------------------------------------------------+
| 1\|c\|\ *approach*   | 1c\|\ *how method is modified*                                      |
+======================+=====================================================================+
| GMT\_VIA\_VECTOR     | The user’s data columns are addressed via a GMT\_VECTOR structure   |
+----------------------+---------------------------------------------------------------------+
| GMT\_VIA\_MATRIX     | The user’s grid is addressed via a GMT\_MATRIX structure            |
+----------------------+---------------------------------------------------------------------+

Integer constants defined for use when user data forms are involved.
These are to be added to the *method* used when registering the
resource. [tbl:via]

[h]

+----------------------+-------------------------------------------+
| 1\|c\|\ *geometry*   | 1c\|\ *description*                       |
+======================+===========================================+
| GMT\_IS\_TEXT        | Not a geographic item                     |
+----------------------+-------------------------------------------+
| GMT\_IS\_POINT       | Multi-dimensional point data              |
+----------------------+-------------------------------------------+
| GMT\_IS\_LINE        | Geographic or Cartesian line segments     |
+----------------------+-------------------------------------------+
| GMT\_IS\_POLYGON     | Geographic or Cartesian closed polygons   |
+----------------------+-------------------------------------------+
| GMT\_IS\_SURFACE     | 2-D gridded surface                       |
+----------------------+-------------------------------------------+

Integer constants defined to register different geometries.
[tbl:geometry]

[h]

\|c\|l\|l\|

2\|c\|\ *Index* & 1c\|\ *content*
0 & XLO & x\_min (west) boundary of grid subset
1 & XHI & x\_max (east) boundary of grid subset
2 & YLO & y\_min (south) boundary of grid subset
3 & YHI & y\_max (north) boundary of grid subset

Domain boundaries (``wesn``) used when selecting subsets of grids.
[tbl:wesn]

Resource initialization
~~~~~~~~~~~~~~~~~~~~~~~

All  programs dealing with (a) input or output files given on the
command line or (b) defaulting to the standard input or output streams
if no files are given, must call the i/o initializer function
``GMT_Init_IO`` once for each direction required (i.e., input and
output). For input it will determine how many input sources have already
been registered. If none have been registered then it will scan the
program arguments for any filenames given on the command line, and
register these input resources. Finally, if we still have found no input
sources we will specify the standard input stream as the single input
source. Likewise, for output: If no single destination has been
registered we specify the standard output stream as the output
destination. Only one output destination is allowed to be active when
the module writes data. The prototype for this function is

::

    long GMT_Init_IO (struct GMTAPI_CTRL *API, long family, long geometry, \
        long direction, long mode, struct GMT_OPTION *head)

where ``family`` specifies what kind of resource is to be registered,
``geometry`` specifies the geometry of the data, the ``direction`` is
either ``GMT_IN`` or ``GMT_OUT``, the ``mode`` is a bit flag that
determines what we do if no resources have been registered. The choices
are

1
    (or GMT\_REG\_FILES\_IF\_NONE) means “add command line (option)
    files if none have been registered already”

2
    (or GMT\_REG\_FILES\_ALWAYS) means “always add any command line
    files”

4
    (or GMT\_REG\_STD\_IF\_NONE) means “add std\* if no other
    input/output have been specified”

8
    (or GMT\_REG\_STD\_ALWAYS) means “always add std\* even if resources
    have been registered”.

The standard behavior is 5 (or GMT\_REG\_DEFAULT). Finally, ``head`` is
the first element of the option structure list.

Many programs will register an export location to hold the results of a
 function (say, a filtered grid), but then wish to use that location as
an *input* resource in the next step. This is accomplished by
re-registering the same array location as an import source, thereby
changing the *direction* of the data set. The function returns TRUE (1)
if there is an error (which is passed back with ``API->error``),
otherwise it returns FALSE (0).

Dimension parameters for user column vectors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We refer to Table [tbl:vector]. The ``type`` array must hold the data
type of each data column in the user’s program. All types other than
GMTAPI\_DOUBLE will need to be converted internally in  to ``double``,
thus possibly increasing memory requirements. If the type is
GMTAPI\_DOUBLE then  will be able to use the column directly by
reference. The ``n_columns`` and ``n_rows`` parameters inform of the
number of vectors and their common length. These are known in the case
of input but may be unknowable in the case of output; if so you may pass
0 for these values and set ``alloc_mode`` to 1; this will make sure
 will allocate the necessary memory at the location you specify.

Dimension parameters for user 2-D table arrays
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We refer to Table [tbl:matrix]. The ``type`` parameter specifies the
data type used for the array in the user’s program. All types other than
GMTAPI\_FLOAT will need to be converted internally in  to ``float``,
thus possibly increasing memory requirements. If the type is
GMTAPI\_FLOAT then  may be able to use the matrix directly by reference.
The ``n_rows`` and ``n_columns`` parameters simply specify the
dimensions of the matrix. These are known in the case of input but may
be unknowable in the case of output; if so you may pass 0 for these
values and set ``alloc_mode`` to 1; this will make sure  will allocate
the necessary memory at the location you specify. Fortran users will
instead have to specify a size large enough to hold the anticipated
output data. The ``registration`` and ``limit`` gives the grid
registration and domain. Finally, use the ``dim`` entry to indicate if
the memory matrix has a dimension that exceeds that of the leading row
(or column) dimension. Note: For GMT\_IS\_TEXTSET the user matrix is
expected to be a 2-D character array with row length by ``dim]`` but we
only consider the first ``n_columns`` characters. For data grids you
will also need to specify the ``registration`` (see the  Cookbook and
Reference, Appendix B for description of the two forms of registration)
and data domain ``limits``.

Create empty resources
----------------------

[sec:create]

If your session needs to build and populate  resources in ways that do
not depend on external resources (files, memory locations, etc.), then
you can obtain a “blank slate” of certain  structures. This is done with
the ``GMT_Create_Data`` function, whose prototype is .

::

    void * GMT_Create_Data (struct GMTAPI_CTRL *API, long family, long par[])

which returns a pointer to the allocated resource. Pass ``family`` as
one of GMT\_IS\_GMTGRID, GMT\_IS\_DATASET, GMT\_IS\_TEXTSET, or
GMT\_IS\_CPT, or the special families GMT\_IS\_VECTOR or GMT\_IS\_MATRIX
when handling user data. Depending on the data type chosen you may need
to pass additional parameters via the ``par`` array, as indicated below:

GMT\_IS\_GMTGRID
    : An empty GMT\_GRID structure with a header is allocated; the data
    array is NULL. The ``par`` argument is not used.

GMT\_IS\_DATASET
    : An empty GMT\_DATASET structure consisting of ``par[0]`` tables,
    each with ``par[1]`` segments, each with ``par[2]`` columns, all
    with ``par[3]`` rows, is allocated.

GMT\_IS\_TEXTSET
    : An empty GMT\_TEXTSET structure consisting of ``par[0]`` tables,
    each with ``par[1]`` segments, all with ``par[2]`` text record, is
    allocated.

GMT\_IS\_CPT
    : An empty GMT\_PALETTE structure with ``par[0]`` palette entries is
    allocated.

GMT\_IS\_VECTOR
    : An empty GMT\_VECTOR structure with ``par[0]`` column entries is
    allocated.

GMT\_IS\_MATRIX
    : An empty GMT\_VECTOR structure is allocated.

In all cases the data entries are initialized to zero (NULL in the case
of text). Note: if you need to duplicate an existing data structure the
simplest way is to use ``GMT_Get_Data`` after registering the original
structure as the data source. The function returns a pointer to the data
container. In case of an error we return a NULL pointer and pass an
error code via ``API->error``.

Import Data
-----------

If your main program needs to read any of the five recognized data types
(CPT files, data tables, text tables,  grids, or images) you will use
the ``GMT_Get_Data`` or ``GMT_Read_Data`` functions, which both returns
entire data sets. In the case of data and text tables, you may also
consider reading record-by-record using the ``GMT_Get_Record`` function.
As a general rule, your program organization will simplify if you can
read the entire resource into memory with ``GMT_Get_Data`` or
``GMT_Read_Data``. However, if this leads to unacceptable waste of
memory or if the program logic is particularly simple, it may be better
to obtain one data record at the time via ``GMT_Get_Record``.

All of these input functions takes a parameter called ``mode``. The
``mode`` parameter generally takes on different meanings for the
different data types and will be discussed below. However, one bit
setting is common to all types: By default, you are only allowed to read
a data source once; the source is then flagged as having been read and
subsequent attempts to read from the same source will result in a
warning and no reading takes place. In the unlikely event you need to
re-read a source you can override this default behavior by adding
GMT\_IO\_RESET to your ``mode`` parameter. Note that this override does
not apply to sources that are streams or file handles.

Enable Data Import
~~~~~~~~~~~~~~~~~~

Once all input resources have been registered, we signal the API that we
are done with the registration phase and are ready to start the actual
data import. This step is only required when reading one record at the
time. We initialize record-by-record reading by calling
``GMT_Begin_IO``. This function enables dataset and text set
record-by-record import and prepares the registered sources for the
upcoming import. The prototype is

::

    long GMT_Begin_IO (struct GMTAPI_CTRL *API, long family, long direction)

where ``family`` specifies what kind of resources is about to be read or
written (see Table [tbl:family] for list of all families; only
GMT\_IS\_DATASET and GMT\_IS\_TEXTSET are available for record-by-record
handling). The ``direction`` is either GMT\_IN or GMT\_out, and for
import we obviously use GMT\_IN. The function determines which is the
first input file and sets up procedures for skipping to the next input
file in a virtual data set. The ``GMT_Get_Record`` function will not be
able to read any data before ``GMT_Begin_IO`` has been called. As you
might guess, there is a companion ``GMT_End_IO`` function that
completes, then disables record-by-record data access. You can use these
several times to switch modes between registering data resources, doing
the importing/exporting, and disabling further data access, perhaps to
do more registration. We will discuss ``GMT_End_IO`` once we are done
with the data import. The function returns TRUE (1) if there is an error
(which is passed back with ``API->error``), otherwise it returns FALSE
(0).

Import a data set
~~~~~~~~~~~~~~~~~

If your main program needs to import any of the five recognized data
types (CPT table, data table, text table,  grid, or image) you will use
either the ``GMT_Read_Data`` or ``GMT_Get_Data`` functions. The former
is typically used when reading from files, streams (e.g., ``stdin``), or
an open file handle, while the latter is only used with a registered
resource via its ID. Because of the similarities of these five import
functions we use an generic form that covers all of them.

Import from a file, stream, or handle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To read an entire data set from a file, a stream, or file handle, use

::

    void * GMT_Read_Data (struct GMTAPI_CTRL *API, long family, long method, \
        long geometry, double wesn[], long mode, char *input, void *data)

where ``data`` is usually NULL except when reading grids in two steps
(i.e., first get a grid structure with a header, then read the data).
Most of these arguments have been discussed earlier. This function can
be called in three different situations:

#. If you have a single source (filename, stream pointer, etc.) you can
   call ``GMT_Read_Data`` directly; there is no need to first register
   the source with ``GMT_Register_IO`` or gather the sources with
   ``GMT_Init_IO``. However, if you did register a single source you can
   still pass it via an encoded filename (see ``GMT_Encode_ID``) or you
   can instead use ``GMT_Get_Data`` using the integer ID directly (see
   next section).

#. If you want to specify ``stdin`` as source then use ``input`` as
   NULL.

#. If you already registered all available sources with ``GMT_Init_IO``
   then you pass ``geometry`` = 0.

Space will be allocated to hold the results, if needed, and a pointer to
the object is returned. If there are errors we simply return NULL and
pass back the error code via ``API->error``. The ``mode`` parameter
takes on different meanings for the different data types.

CPT table
    : ``mode`` are bit-flags that controls how the CPT file’s back-,
    fore-, and NaN-colors should be initialized. Select 0 to read the
    CPT file’s back-, fore-, and NaN-colors, 2 to replace these with the
     default values, or 4 to replace them with the color tables entries
    for highest and lowest value.

Data table
    : ``mode`` is not used.

Text table
    : ``mode`` is not used.

GMT grid
    : Here ``mode`` determines how we read the grid: To get the entire
    grid and its header, pass GMT\_GRID\_ALL. However, if you need to
    extract a sub-region you must first get the header only by passing
    GMT\_GRID\_HEADER, then use the header structure attributes
    ``wesn``, to specify a subset via the array ``wesn``, and then call
    ``GMT_Read_Data`` a second time, with ``mode`` = GMT\_GRID\_DATA and
    passing your ``wesn`` array and the grid structure returned from the
    first call. In the event your data array should be allocated to hold
    both the real and imaginary parts of a complex data set you must add
    either GMT\_GRID\_COMPLEX\_REAL or GMT\_GRID\_COMPLEX\_IMAG to
    ``mode`` so as to allow for the extra space and to position the
    input values correctly.

Import from a memory location
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

However, if you are importing from memory locations or prefer to first
register the source, then you should use ``GMT_Get_Data`` instead. This
function requires fewer arguments since you simply pass the unique ID
number of the resource. The function is described as follows:

::

    void * GMT_Get_Data (struct GMTAPI_CTRL *API, long ID, long mode, \
        void *data)

The ``ID`` is the unique object ID you received when registering the
resource earlier, ``mode`` controls some aspects of the import (see
``GMT_Read_Data`` above), while ``data`` is usually NULL except when
reading grids in two steps (i.e., first get a grid structure with a
header, then read the data). Most of the other arguments have been
discussed earlier. Space will be allocated to hold the results, if
needed, and a pointer to the object is returned. If there are errors we
simply return NULL and pass back the error code via ``API->error``.

Retrieve an allocated result
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Finally, if you need to access the result that a GMT module normally
will write to an output file, then you need to register an output
destination with ``GMT_Register_IO`` first (passing ``ptr`` == NULL).
The GMT module will then allocate space to hold the output and let the
API know where this memory resides. You can then use
``GMT_Retrieve_Data`` to get a pointer to the container where the data
was stored. This function requires fewer arguments since you simply pass
the unique ID number of the resource. The function is described as
follows:

::

    void * GMT_Retrieve_Data (struct GMTAPI_CTRL *API, long ID)

The ``ID`` is the unique object ID you received when registering the
NULL resource earlier, Since this container has already been created, a
pointer to the object is returned. If there are errors we simply return
NULL and pass back the error code via ``API->error``.

Importing a data record
~~~~~~~~~~~~~~~~~~~~~~~

If your program must read data table records one-by-one you must first
enable this input mechanism with ``GMT_Begin_IO`` and then use the
``GMT_Get_Record`` function in a loop; the prototype is

::

    void * GMT_Get_Record (struct GMTAPI_CTRL *API, long mode, long *nfields)

where the returned value is either a pointer to a double array with the
current row or a pointer to a character string with the current row,
depending on ``mode``. In either case these pointers point to memory
internal to  and should be considered read-only. When we reach
end-of-file, encounter conversion problems, read header comments, or
identify segment headers we return a NULL pointer, with the status of
the current record returned via ``API->GMT->current.io.status``.
Typically, this status is examined using macros that return TRUE or
FALSE depending on the particular check. There are 11 macros available
to programmers; for a list see Table [tbl:iomacros]. The ``nfields``
pointer will return the number of fields read; pass NULL if your program
does not need this information.

Normally (``mode`` == GMT\_READ\_DOUBLE or 0), we return a pointer to
the double array. To read text record, supply instead ``mode`` ==
GMT\_READ\_TEXT (or 1) and we instead return a pointer to the text
record. However, if you have input records that mixes organized
floating-point columns with text items you could pass ``mode`` ==
GMT\_READ\_MIXED (2). Then,  will attempt to extract the floating-point
values; you can still access the record string, as discussed below.
Finally, if your application needs to be notified when  closes one file
and opens another, add GMT\_FILE\_BREAK to ``mode`` and check for the
return code GMT\_IO\_NEXT\_FILE (By default, we treat the combination of
many input files as one virtual file). Using ``GMT_Get_Record`` requires
you to first initialize the source(s) with ``GMT_Init_IO``. This
function returns NULL if there are problems and sets status codes that
your program will need to examine to take appropriate response:

GMT\_IO\_TABLE\_HEADER
    : We read a table header; to examine this text string (if working
    with ASCII data), see ``API->GMT->current.io.segment_header``.

GMT\_IO\_SEGMENT\_HEADER
    : We read a segment header; to examine this text string (if working
    with ASCII data), see ``API->GMT->current.io.current_record``.

GMT\_IO\_MISMATCH
    : The number of columns read is less than what the program expected.

GMT\_IO\_EOF
    : We have reached the end of the source.

GMT\_IO\_NAN
    : The record has NaNs in fields that we do not allow to have NaNs,
    and hence, it is a bad record (see ’s IO\_NAN\_RECORD defaults).

GMT\_IO\_GAP
    : A user-defined data gap has been encountered (see ’s g option)

Developers who need to import data on a record-by-record basis should
consult the source code of, say, blockmean\_func.c or pstext\_func.c.

[h]

+-----------------------------------+-------------------------------------------------------------+
| 1\|c\|\ *Macro*                   | 1c\|\ *description*                                         |
+===================================+=============================================================+
| ``GMT_REC_IS_TBL_HEADER(API)``    | TRUE if we read a table header                              |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_SEG_HEADER(API)``    | TRUE if we read a segment header                            |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_ANY_HEADER(API)``    | TRUE if we read either header type                          |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_ERROR(API)``         | TRUE if we had a read or conversion failure                 |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_EOF(API)``           | TRUE if we reached the end of the file (EOF)                |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_NAN(API)``           | TRUE if we only read NaNs                                   |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_GAP(API)``           | TRUE if this record implies a data gap                      |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_NEW_SEGMENT(API)``   | TRUE if we enter a new segment                              |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_LINE_BREAK(API)``    | TRUE if we encountered a segment header, EOF, NaNs or gap   |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_FILE_BREAK(API)``    | TRUE if we finished one file but not the last               |
+-----------------------------------+-------------------------------------------------------------+
| ``GMT_REC_IS_DATA(API)``          | TRUE if we read a data record                               |
+-----------------------------------+-------------------------------------------------------------+

Macros used to determine status of current data record. The gap macro
depends on the current g settings. [tbl:iomacros]

Disable Data Import
~~~~~~~~~~~~~~~~~~~

Once the record-by-record input processing has completed we disable
further input to prevent accidental reading from occurring (due to poor
program structure, bugs, etc.). We do so by calling ``GMT_End_IO``. This
function disables further record-by-record data import; its prototype is

::

    long GMT_End_IO (struct GMTAPI_CTRL *API, long direction, long mode)

and we specify ``direction`` = GMT\_IN. At the moment, ``mode`` is not
used. This call will also reallocate any arrays obtained into their
proper lengths. The function returns TRUE (1) if there is an error
(which is passed back with ``API->error``), otherwise it returns FALSE
(0).

Prepare program options
-----------------------

[sec:func] The module prototype interface is

::

    long GMT_module (struct GMTAPI_CTRL *API, long mode, void *args)

All GMT modules may be called with one of three sets of ``args``
depending on ``mode``. The three modes differ in how the options are
passed to the module:

:math:`mode > 0`
    : Expects ``args`` to be an array of text options ``mode`` to be a
    count of how many options are passed (i.e., the ``argc, argv[]``
    model).

:math:`mode < 0`
    : Expects ``args`` to be a pointer to a doubly-linked list of
    objects with individual options for the current program.

:math:`mode == 0`
    : Expects ``args`` to be a single text string with all required
    options.

Here, ``GMT_module`` stands for any of the  modules, such as
``GMT_psxy``. All modules returns FALSE (o) if they returned
successfully; otherwise they return an error code back to the calling
environment.

Set program options via text array arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When textttmode :math:`> 0` we expect an array ``args`` of character
strings that each holds a single command line argument (e.g.,
“R\ *120:30/134:45/8S/3N*”) and interpret ``mode`` to be the count of
how many options are passed. This, of course, is almost exactly how the
stand-alone programs are called (and reflects how they themselves are
activated internally). We call this the “argc–argv” mode. Depending on
how your program obtains the necessary options you may find that this
interface offers all you need.

Set program options via text command
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If ``mode`` == 0 then ``args`` will be examined to see if it contains
several options within a single command string. If so we will break
these into separate options. This is useful if you wish to pass a single
string such as “R\ *120:30/134:45/8S/3N* JM\ *6i* mydata.txt Sc0.2c”. We
call this the “command” mode.

Set program options via linked structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The third, linked-list interface allows developers using higher-level
programming languages to pass all command options via a pointer to a
NULL-terminated, doubly-linked list of option structures, each
containing information about a single option. Here, instead of text
arguments we pass the pointer to the linked list of options mentioned
above, and ``mode`` must be passed as -1 (or any negative value). Using
this interface can be more involved since you need to generate the
linked list of program options; however, utility functions exist to
simplify its use. This interface is intended for programs whose internal
workings are better suited to generate such arguments – we call this the
“options” mode. The order in the list is not important as  will sort it
internally according to need. The option structure is defined in Table
[tbl:options].

[h]

ll

2l\ ``struct GMT_OPTION {``
``char option;`` & /\* *Single character of the option (e.g.,’G’ for* G
\*/
``char *arg;`` & /\* *String pointer with arguments (NULL if not used)*
\*/
``struct GMT_OPTION *next;`` & /\* *Pointer to next option (NULL for
last option)* \*/
``struct GMT_OPTION *prev;`` & /\* *Pointer to previous option (NULL for
first option)* \*/
``};`` &

Definition of the structure used to hold a single program option.
[tbl:options]

Convert between text and linked structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To assist programmers there are also two convenience functions that
allow you to convert between the two argument formats. They are

::

    struct GMT_OPTIONS * GMT_Create_Options (struct GMTAPI_CTRL *API, \
        long argc, void *args)

This function accepts your array of text arguments (cast via a void
pointer), allocates the necessary space, performs the conversion, and
returns a pointer to the head of the linked list of program options.
However, in case of an error we return a NULL pointer and set
``API->error`` to indicate the nature of the problem. Otherwise, the
pointer may now be passed to the relevant ``GMT_module``. Note that if
your list of text arguments were obtained from a C ``main()`` function
then ``argv[0]`` will contain the name of the calling program. To avoid
passing this as a file name option, call ``GMT_Create_Options`` with
``argc-1`` and ``argv+1``. If, you wish to pass a single text string
with multiple options (in lieu of an array of text strings), then pass
``arg`` = 0. When no longer needed you can remove the entire list by
calling

::

    long GMT_Destroy_Options (struct GMTAPI_CTRL *API, \
        struct GMT_OPTION **list)

The function returns TRUE (1) if there is an error (which is passed back
with ``API->error``), otherwise it returns FALSE (0).

The inverse function prototype is

::

    char ** GMT_Create_Args (struct GMTAPI_CTRL *API, long *argc, \
        struct GMT_OPTIONS *list)

which allocates space for the text strings and performs the conversion;
it passes back the count of the arguments via ``argc`` and returns a
pointer to the text array. In the case of an error we return a NULL
pointer and set ``API->error`` to reflect the error type. Note that
``argv[0]`` will not contain the name of the program as is the case the
arguments presented by a C ``main()`` function. When you no longer have
any use for the text array, call

::

    long GMT_Destroy_Args (struct GMTAPI_CTRL *API, long argc, char *argv[])

to deallocate the space used. This function returns TRUE (1) if there is
an error (which is passed back with ``API->error``), otherwise it
returns FALSE (0).

Finally, to convert the linked list of option structures to a single
text string command, use

::

    char * GMT_Create_Cmd (struct GMTAPI_CTRL *API, struct GMT_OPTION *list)

Developers who plan to import and export  shell scripts might find it
convenient to use these functions. In case of an error we return a NULL
pointer and set ``API->error``, otherwise a pointer to an allocated
string is returned. It

Manage the linked list of options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Several additional utility functions are available for programmers who
wish to manipulate program option structures within their own programs.
These allow you to create new option structures, append them to the
linked list, replace existing options with new values, find a particular
option, and remove options from the list. Note: The order in which the
options appear in the linked list is of no consequence to . Internally,
 will sort and process the options in the manner required. Externally,
you are free to maintain your own order.

Make a new option structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^

``GMT_Make_Option`` will allocate a new option structure, assign it
values given the ``option`` and ``arg`` parameter (pass NULL if there is
no argument for this option), and returns a pointer to the allocated
structure. The prototype is

::

    struct GMT_OPTION *GMT_Make_Option (struct GMTAPI_CTRL *API, char option, \
        char *arg)

Should memory allocation fail the function will print an error message
set an error code via ``API->error``, and return NULL.

Append an option to the linked list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``GMT_Append_Option`` will append the specified ``option`` to the end of
the doubly-linked ``list``. The prototype is

::

    struct GMT_OPTION * GMT_Append_Option (struct GMTAPI_CTRL *API, \
        struct GMT_OPTION *option, struct GMT_OPTION *list)

We return the list back, and if ``list`` is given as NULL we return
``option`` as the start of the new list. Any errors results in a NULL
pointer with ``API->error`` holding the error type.

Find an option in the linked list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``GMT_Find_Option`` will return a pointer ``ptr`` to the first option in
the linked list starting at ``list`` whose option character equals
``option``. If not found we return NULL. While this is not necessarily
an error we still set ``API->error`` accordingly. The prototype is

::

    struct GMT_OPTION *GMT_Find_Option (struct GMTAPI_CTRL *API, char option, \
        struct GMT_OPTION *list)

If you need to look for multiple occurrences of a certain option you
will need to call ``GMT_Find_Option`` again, passing the option
following the previously found option as the ``list`` entry, i.e.,

::

    list = *ptr->next;

Update an existing option in the list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``GMT_Update_Option`` will first determine if ``option`` exists; if so
it will delete it. Then, it will make a new option from the arguments
and append it to the end of the linked ``list``. The prototype is

::

    long GMT_Update_Option (struct GMTAPI_CTRL *API, char option, \
        char *arg, struct GMT_OPTION *list)

An error will be reported if (a) ``list`` is NULL or (b) the option is
not found. The function returns TRUE (1) if there is an error (i.e.,
``list`` is NULL or the option is not found); the error code is passed
back via ``API->error``. Otherwise it returns FALSE (0).

Delete an existing option in the linked list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You may use ``GMT_Delete_Option`` to remove ``option`` from the linked
``list``. The prototype is

::

    long GMT_Delete_Option (struct GMTAPI_CTRL *API, \
        struct GMT_OPTION *current)

We return TRUE if the option is not found in the list and set
``API->error`` accordingly. Note: Only the first occurrence of the
specified option will be deleted. If you need to delete all such options
you will need to call this function in a loop until it returns a
non-zero status.

Specify a file via an linked option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To specify an input file name via an option, simply use :math:`<` as the
option (this is what ``GMT_Create_Options`` does when it finds filenames
on the command line). Likewise, :math:`>` can be used to explicitly
indicate an output file. In order to append to an existing file, use
:math:`>>`. For example the following command would read from file.A and
append to file.B:

::

    gmtconvert -<file.A ->>file.B

These options also work on the command line but usually one would have
to escape the special characters :math:`<` and :math:`>` as they are
used for file redirection.

Parsing GMT common options
~~~~~~~~~~~~~~~~~~~~~~~~~~

While all the main  modules have their own specific option parser, we
also provide a general parser that only examines the common  options
such as R, J, V, etc. The prototype of this parser is

::

    long GMT_Parse_Common (struct GMTAPI_CTRL *API, \
        struct GMT_OPTION *list)

An error will be reported via ``API->error`` if any of the common
 options fail to parse, and if so we return TRUE; if not errors we
return FALSE. All other options, including file names, will be silently
ignored. The parsing will update the internal information structure that
affects program operations.

Calling a GMT module
--------------------

Given your linked list of program options (or text array) and possibly
some registered resources, you can now call the required  module using
one of the two flavors discussed in section [sec:func]. All modules
return an error or status code that your program should consider before
processing the results.

Exporting Data
--------------

If your program needs to write any of the four recognized data types
(CPT files, data tables, text tables, or  grids) you can use the
``GMT_Put_Data``. In the case of data and text tables, you may also
consider the ``GMT_Put_Record`` function. As a general rule, your
program organization may simplify if you can write the export the entire
resource with ``GMT_Put_Data``. However, if the program logic is simple
or already involves using ``GMT_Get_Record``, it may be better to export
one data record at the time via ``GMT_Put_Record``.

Both of these output functions takes a parameter called ``mode``. The
``mode`` parameter generally takes on different meanings for the
different data types and will be discussed below. However, one bit
setting is common to all types: By default, you are only allowed to
write a data resource once; the resource is then flagged to have been
written and subsequent attempts to write to the same resource will
quietly be ignored. In the unlikely event you need to re-write a
resource you can override this default behavior by adding GMT\_IO\_RESET
to your ``mode`` parameter.

Enable Data Export
~~~~~~~~~~~~~~~~~~

Similar to the data import procedures, once all output destinations have
been registered, we signal the API that we are done with the
registration phase and are ready to start the actual data export. As for
input, this step is only needed when dealing with record-by-record
writing. Again, we enable record-by-record writing by calling
``GMT_Begin_IO``, this time with ``direction`` = GMT\_OUT. This function
enables data export and prepares the registered destinations for the
upcoming writing.

Exporting a data set
~~~~~~~~~~~~~~~~~~~~

To have your program accept results from  modules and write them
separately requires you to use the ``GMT_Write_Data`` or
``GMT_Put_Data`` functions. They are very similar to the
``GMT_Read_Data`` and ``GMT_Get_Data`` functions encountered earlier.

Exporting a data set to a file, stream, or handle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The prototype for writing to a file (via name, stream, or file handle)
is

::

    long GMT_Write_Data (struct GMTAPI_CTRL *API, long family, long method, \
        long geometry, double wesn[], long mode, void *output, void *data)

where ``data`` is a pointer to any of the four structures discussed
previously. Again, the ``mode`` parameter is specific to each data type:

CPT table
    : ``mode`` controls if the CPT table’s back-, fore-, and NaN-colors
    should be written (1) or not (0).

Data table
    : If ``method`` is GMT\_IS\_FILE, then the value of ``mode`` affects
    how the data set is written:

    GMT\_WRITE\_DATASET
        : The entire data set will be written to the single file [0].

    GMT\_WRITE\_TABLES
        : Each table in the data set is written to individual files [1].
        You can either specify an output file name that *must* contain
        one C-style format specifier for a long variable (e.g.,
        “New\_Table\_%6.6ld.txt”), which will be replaced with the table
        number (a running number from 0) *or* you must assign to each
        table *i* a unique output file name via the
        ``D->table[i]->file[GMT_OUT]`` variables prior to calling the
        function.

    GMT\_WRITE\_SEGMENTS
        : Each segment in the data set is written to an individual file
        [2]. Same setup as for GMT\_WRITE\_TABLES except we use
        sequential segment numbers to build the file names.

    GMT\_WRITE\_TABLE\_SEGMENTS
        : Each segment in the data set is written to an individual file
        [3]. You can either specify an output file name that *must*
        contain two C-style format specifiers for two long variables
        (e.g., “New\_Table\_%6.6ld\_Segment\_%3.3ld.txt”), which will be
        replaced with the table and segment numbers, *or* you must
        assign to each segment *j* in each table *i* a unique output
        file name via the ``D->table[i]->segment[j]->file[GMT_OUT]``
        variables prior to calling the function.

    GMT\_WRITE\_OGR
        : Writes the dataset in OGR/GMT format in conjunction with the a
        setting [4].

Text table
    : The ``mode`` is used the same way as for data tables.

GMT grid
    : Here, ``mode`` may be GMT\_GRID\_HEADER to only update a file’s
    header structure, but normally it is simply GMT\_GRID\_ALL (0) so
    the entire grid and its header will be exported (a subset is not
    allowed during export). However, in the event your data array holds
    both the real and imaginary parts of a complex data set you must add
    either GMT\_GRID\_COMPLEX\_REAL (4) or GMT\_GRID\_COMPLEX\_IMAG (16)
    to ``mode`` so as to export the corresponding grid values correctly.
    Finally, for native binary grids you may skip writing the grid
    header by adding GMT\_GRID\_NO\_HEADER (16); this setting is ignored
    for other grid formats.

If successful the function returns FALSE (0); otherwise we return TRUE
(1) and set ``API->error`` to reflect to cause.

Exporting a data set to memory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If writing to a memory destination you will want to first register that
destination and then use the returned ID with ``GMT_Put_Data`` instead:

::

    long GMT_Put_Data (struct GMTAPI_CTRL *API, long ID, long mode, \
        void *data)

where ``ID`` is the unique ID of the registered destination, ``mode`` is
specific to each data type (and controls aspects of the output
structuring), and ``data`` is a pointer to any of the four structures
discussed previously. For more detail, see ``GMT_Write_Data`` above. If
successful the function returns FALSE (0); otherwise we return TRUE (1)
and set ``API->error`` to reflect to cause.

Exporting a data record
~~~~~~~~~~~~~~~~~~~~~~~

If your program must write data table records one-by-one you must first
enable record-by-record writing with ``GMT_Begin_IO`` and then use the
``GMT_Put_Record`` function in a loop; the prototype is

::

    long GMT_Put_Record (struct GMTAPI_CTRL *API, long mode, void *rec)

where ``rec`` is a pointer to either (a) a double-precision array with
the current row. Then, ``rec`` is expected to hold at least as many
items as the current setting of ``n_col[GMT_OUT]``, which represents the
number of columns in the output destination. Alternatively (b), ``rec``
points to a text string. The ``mode`` parameter must be set to reflect
what is passed. Using ``GMT_Put_Record`` requires you to first
initialize the destination with ``GMT_Init_IO``. Note that for families
GMT\_IS\_DATASET and GMT\_IS\_TEXTSET the methods GMT\_IS\_COPY and
GMT\_IS\_REF are not supported since you can simply populate the
GMT\_DATASET structure directly. As mentioned, ``mode`` affects what is
actually written:

GMT\_WRITE\_DOUBLE
    : Normal operation that builds the current output record from the
    values in ``rec`` [0].

GMT\_WRITE\_TEXT
    : For ASCII output mode we write the text string ``rec``. If ``rec``
    is NULL then we use the current (last imported) text record. If
    binary output mode we quietly skip writing this record [1].

GMT\_WRITE\_TBLHEADER
    : For ASCII output mode we write the text string ``rec``. If ``rec``
    is NULL then we write the last read header record (and ensures it
    starts with #). If binary output mode we quietly skip writing this
    record [2].

GMT\_WRITE\_SEGHEADER
    : For ASCII output mode we use the text string ``rec`` as the
    segment header. If ``rec`` is NULL then we use the current (last
    read) segment header record. If binary output mode instead we write
    a record composed of NaNs [1].

The function returns TRUE (1) if there was an error associated with the
writing (which is passed back with ``API->error``), otherwise it returns
FALSE (0).

Disable Data Export
~~~~~~~~~~~~~~~~~~~

Once the record-by-record output has completed we disable further output
to prevent accidental writing from occurring (due to poor program
structure, bugs, etc.). We do so by calling ``GMT_End_IO``. This
function disables further record-by-record data export; here, we
obviously pass ``direction`` = GMT\_OUT.

Destroy allocated resources
---------------------------

If your session imported any data sets into memory then you may
explicitly free this memory once it is no longer needed and before
terminating the session. This is done with the ``GMT_Destroy_Data``
function, whose prototype is

::

    long GMT_Destroy_Data (struct GMTAPI_CTRL *API, long mode, void *data)

where ``data`` is the address of the pointer to a data container. Pass
``mode`` either as GMT\_ALLOCATED or GMT\_REFERENCE. The former is used
internally by the  modules since they can only free resources that are
not destined to live on in the memory of their calling program. The
latter mode is used to free resources in your calling program. Note that
when each module completes it will automatically free memory created by
the API; similarly, when the session is destroyed we also automatically
free up memory. Thus, ``GMT_Destroy_Data`` is therefore generally only
needed when you wish to directly free up memory to avoid running out of
it. The function returns TRUE (1) if there is an error when trying to
free the memory (the error code is passed back with ``API->error``),
otherwise it returns FALSE (0).

Terminate a GMT session
-----------------------

Before your program exits it should properly terminate the  session,
which involves a call to

::

    long GMT_Destroy_Session (struct GMTAPI_CTRL **API)

which simply takes the address of the pointer to the  API control
structure as its only arguments. It terminates the  machinery with a
call to ``GMT_end`` and deallocates all memory used by the  API
book-keeping. If you requested PSL during creation then the PSL
resources are freed as well. It also unregisters any remaining resources
previously registered with the session. The  API will only close files
that it was responsible for opening in the first place. Finally, the API
structure itself is freed so your main program does not need to do so.
The function returns TRUE (1) if there is an error when trying to free
the memory (the error code is passed back with ``API->error``),
otherwise it returns FALSE (0).

Report errors
-------------

Since all API functions returns a status code via ``API->error``, you
should always check this code before moving to the next step. All API
functions will issue an error message before returning control to the
calling program. This function is

::

    long GMT_Report_Error (struct GMTAPI_CTRL *API, long error);

where ``error`` is the status code return by any API function. Note: The
error message is only issued if the verbosity level of the  session is
not set to 0 [Default is 1], and messages are normally written to
``stderr`` unless this stream has been redirected. Note that this
function also updates ``API->error`` to the given value.

FORTRAN 77 interface
--------------------

FORTRAN 77 developers who wish to use the  API may use the same 17 API
functions as discussed in Chapter 2. However, as pointers to structures
and such are not available, the FORTRAN bindings provided simplifies the
interface in two ways:

-  The first argument to the functions (the GMTAPI Control structure
   pointer) is not provided. Instead, the bindings use a hidden, global
   external structure for this purpose and pass the pointer to it down
   to the C version of the functions.

-  The resource arguments in ``GMT_Register_IO`` are not pointers to
   items but the items themselves.

The list of the basic 17 FORTRAN prototype functions thus becomes

::

    function GMT_Create_Session (tag, mode)
    function GMT_Destroy_Session ()
    function GMT_Register_IO (family, method, geometry, direction, \
        resource, wesn)
    function GMT_Encode_ID (filename, ID)
    function GMT_Init_IO (family, geometry, direction, head)
    function GMT_Begin_IO (family, geometry, direction)
    function GMT_Create_Data (family, geometry, ipar)
    function GMT_Read_Data (family, method, geometry, wesn, mode, \
        input, data)
    function GMT_Get_Data (ID, mode, data)
    function GMT_Retrieve_Data (ID)
    function GMT_Get_Record (rec, mode, nfields)
    function GMT_Write_Data (family, method, geometry, wesn, mode, \
        output, data)
    function GMT_Put_Data (ID, mode, data)
    function GMT_Put_Record (mode, rec)
    function GMT_End_IO (direction, mode)
    function GMT_Destroy_Data (mode, ptr)
    function GMT_Report_Error (error)

where ``method``, ``geometry``, ``direction``, ``ID`` and ``error`` are
integers, ``ipar`` is an integer parameter array, ``wesn`` is a real
(double precision) array, and ``resource`` are source or destination
addresses.

headings headings

.. [1]
   Currently, only C/C++ and Matlab are being tested.

.. [2]
   However, there is no thread-support yet.

.. |image| image:: GMT4_mode.png
.. |image1| image:: GMT5_mode.png
