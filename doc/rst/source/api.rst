.. set default highlighting language for this document:
.. highlight:: c

.. _api:

=========
GMT C API
=========

Introduction
============

.. index:: ! API

Preamble
--------

.. figure:: /_images/GMT4_mode.png
   :height: 554 px
   :width: 1122 px
   :align: center
   :scale: 50 %

   GMT 4 programs contain all the high-level functionality.


Prior to version 5, the bulk of GMT functionality was coded directly
in the standard GMT C program modules (e.g., ``surface.c``, ``grdimage.c``, etc.). The
GMT library only offered access to low-level functions from which
those high-level GMT programs were built. The standard GMT programs
have been very successful, with tens of thousands of users world-wide.
However, the design of the main programs prevented developers from
leveraging GMT functionality from within other programming
environments since access to GMT tools could only be achieved via
system calls [1]_. Consequently, all data i/o had to be done via
temporary files. The design also prevented the GMT developers
themselves from taking advantage of these modules directly. For
instance, the tool :doc:`legend` needed to
make extensive use of system calls to :doc:`plot` and
:doc:`text` in order to plot the lines,
symbols and text that make up a map legend, making it a very awkward
program to maintain.

.. figure:: /_images/GMT5_mode.png
   :height: 399 px
   :width: 1116 px
   :align: center
   :scale: 50 %

   GMT 5 programs contain all the high-level functionality.


Starting with GMT version 5, all standard GMT programs have been
rewritten into separate function "modules" invoked by a single
driver program called ``gmt.c``.
The :doc:`gmt` executable simply calls the corresponding
GMT modules; it is these modules that do all the work. These new
functions have been placed in a new GMT high-level API library and can
be called from a variety of environments (C/C++, Fortran, Julia, Python,
MATLAB, Visual Basic, R, etc.) [2]_. For example, the main
program ``blockmean.c`` has been reconfigured as a high-level function
``GMT_blockmean()``, which does the actual spatial averaging and can
pass the result back to the calling program (or write it to file). The
previous behavior of ``blockmean.c`` is achieved by calling ``gmt blockmean``,
i.e., the module is now just the first argument to the :doc:`gmt` executable.
For backwards compatibility with older GMT (4) scripts we optionally
install numerous symbolic links to the gmt executable with names such
as blockmean, plot, surface, etc.  The gmt executable is smart enough to
understand when it is being invoked via one of these links and then knows
which module to call upon.
Consequently, ``blockmean.c`` and other files do in
fact no longer exist.

.. figure:: /_images/GMT5_external.png
   :height: 616 px
   :width: 1193 px
   :align: center
   :scale: 50 %

   GMT 5 API showing current and future external environments.


The i/o abstraction layer
-------------------------

In order for the API to be as flexible as possible we have
generalized the notions of input and output. Data that already reside in
an application's memory may serve as input to a GMT module and we refer
to such data as "Virtual Files". Other
sources of input may be file pointers and file descriptors (as well as
the standard mechanism for passing file names). For standard
data table i/o, the GMT API takes care of the task of assembling any
combination of files, pointers, and memory locations into *a single
virtual data set* from which the GMT module may read (a) all
records at once into memory, or (b) read one record at a time. Likewise,
GMT functions may write their output to a virtual destination, which
might be a memory location in the user's application (another Virtual File), a file pointer or
descriptor, or an output file. The GMT modules are unaware of these
details and simply read from a "source" and write to a "destination".
Thus, the standard concept of file-based input/output so familiar to
any GMT user carries over to the API, except for the generalization
that files can be virtual files already in memory.  Because of this
design we will see that we need to associate these virtual files
with special filenames that we may pass to modules, and the modules
will faithfully treat these as real files.  However, under the hood
the API layer will take care of the differences between real and
virtual files.

Users who wish to maintain their own data types and memory management
can also use the GMT modules, but some limitations and requirements do
apply: The user's data can either be provided as (1) a 2-D matrix (of
any data type, e.g., float, integer, etc.) and in any memory layout
configuration (e.g., row-major or column-major layout) or as (2) a
set of column vectors that each may be of any type.  These custom arrays
will need to be hooked onto the GMT containers :ref:`GMT_MATRIX <struct-matrix>`
and :ref:`GMT_VECTOR <struct-vector>`, respectively.
Such objects can then be treated as virtual files for either input of output.

Our audience
------------

Here, we document the new functions in the GMT API library for
application developers who wish to call these functions from their own
custom programs. At this point, only the new high-level GMT API is
fully documented and intended for public use. The structure and
documentation of the under-lying lower-level GMT library is not
finalized. Developers using these functions may risk disruption to their
programs due to changes we may make in the library in support of the
GMT API. However, developers who wish to make supplemental packages to
be distributed as part of GMT will (other than talk to us) probably
want to access the entire low-level GMT library as well. It is
unlikely that the low-level library will ever be fully documented.

There are two classes of development that users can pursue:

#. Building stand-alone custom executables that link with the shared GMT
   API.  Our examples in this documentation are of this kind.  There programs
   are likely to address a user's special data formats or processing needs
   by leveraging high-level GMT modules to do some of the heavy lifting.

#. Building shared library plugins to extend the breath of GMT.  Users who
   wish to build one or more new modules and distributed then via a plugin
   that is dynamically loaded at run-time can now do so.   At the present,
   all the modules in the official GMT supplement are compiled into a single
   plugin that can be accessed at run-time.  Similarly, developers may add
   additional plugin libraries with any number of GMT-like modules and these
   will then be available from the gmt command (as well as from derived
   interfaces such as the GMT/MATLAB toolbox and the Python module).  An
   example of plugin development is given by the
   `GSFML extension to GMT <http://www.soest.hawaii.edu/PT/GSFML/>`_.

Definitions
-----------

For the purpose of this documentation a few definitions are needed:

#. "Standard GMT program" refers to one of the traditional stand-alone
   command-line executables known to all GMT users, e.g.,
   :doc:`blockmean`, :doc:`plot`,
   :doc:`grdimage`, etc. Prior to version 5,
   these were the only GMT executables available.  In GMT 5 and up, these are
   accessed via the :doc:`gmt` executable.

#. "\ GMT module" refers to the function in the GMT API library that
   is responsible for all the action taken by the corresponding
   standard GMT program. All such modules are given the same names as the
   corresponding programs e.g., "blockmean", but are invoked via the
   ``GMT_Call_Module`` function.

#. "\ GMT application" refers to a new application written by any
   developer.  It uses the API, perhaps for custom i/o, and may call one
   or more GMT functions to create a new GMT-compatible executable.

#. "\ GMT plugin library" refers to a collection of one or more new custom
   GMT-like modules that are presented as a plugin library.  It such libraries
   are placed in the official GMT plugin directory or their path is added to
   the GMT defaults parameter :term:`GMT_CUSTOM_LIBS` then the :doc:`gmt` executable can find them.

#. "Family" refers to one of the many high-level GMT data types (e.g., grids, CPTs)
   and is typically a required argument to some API functions.

#. "Method" refers to one of several ways in which data can be read or written
   in the API, including from existing memory variables.

#. "Direction" is typically either GMT_IN (for reading) or GMT_OUT (for writing).

#. In the API description that follows we will use the type ``int`` to
   mean a 4-byte integer. All integers used in the API are 4-byte
   integers with the exception of one function where an 8-byte integer is
   used. Since different operating systems have their own way of
   defining 8-byte integers we use C99's ``int64_t`` for this purpose;
   it is guaranteed to yield the correct type that the GMT function
   expects.

In version 5, the standard GMT programs are themselves simple invocations
of the :doc:`gmt` application with the function name as argument.
However, some of these modules, such as
:doc:`legend`, :doc:`gmtconvert`,
:doc:`grdblend`,
:doc:`grdfilter` and others may call several additional modules.

API changes from GMT5 to GMT 6
------------------------------

The API released with GMT5 was considered experimental as our usage of it in GMT proper
as well as in the GMT/MATLAB toolbox and the GMT/Python package would undoubtably lead to
revisions.  We developed API to enable GMT access from other environments hence we want
the library to address the needs of such developers.  Here are the changes in the GMT 6
API that are not backwards compatible with GMT 5:

#. There is no longer a GMT_TEXTSET resource.  Data records are now generalized to
   contain an optional leading numerical array followed by an optional trailing text.
   A "TEXTSET" in this context is simply a DATASET that has no leading numerical array.
   This change was necessary so that all modules reading tables expect the same fundamental
   GMT_DATASET resource.  The alternative (which we lived to regret) was that developers
   calling modules from their environment would have to format their data in different ways
   depending on the module, and in some case depending on module options.  Now, all table
   modules expect GMT_DATASET.
#. The function GMT_Alloc_Segment no longer takes the family of the segment (since there are
   now only DATASET segments) but the family variable has been reused as a mode which is
   passed as either GMT_WITH_STRINGS or GMT_NO_STRINGS so that data segments can be allocated
   with or without the optional string array.
#. We introduce a new structure GMT_RECORD which is used by GMT_Get_Record and GMT_Put_Record.
   Because such records may have both leading numerical columns and a trailing string these
   functions needed to work with such a structure rather than either an array or string.
#. The unused function GMT_Set_Columns needed to accept *direction* so it could be used for
   either input or output.  It is rarely needed but some tools that must only read *N* numerical
   columns and treat anything beyond that as trailing text (even if numbers) must set the
   fixed input columns before reading.  We also added one more mode (GMT_COL_FIX_NO_TEXT) to
   enforce reading of a fixed number of numerical columns and skip any trailing text.
#. The GMT_DATASET structure has gained a new (hidden) enum GMT_enum_read ``type`` which indicates what
   record types were read to produce this dataset (GMT_READ_DATA, GMT_READ_TEXT, GMT_READ_MIXED).
   We also changed the geometry from unsigned int to enum GMT_enum_geometry.
#. The long obsolete enums GMT_READ_DOUBLE and GMT_WRITE_DOUBLE have now fully been removed;
   use GMT_READ_DATA and GMT_WRITE_DATA instead.
#. The GMT_Convert_Data function's flag array is now of length 2 instead of 3 (because there are no
   longer any TEXTSET settings), with what used to be flag3 now being given as flag2.

GMT resources
-------------

The GMT API knows how to create, duplicate, read and write six types of data objects common to
GMT operations: Pure data tables (ASCII or binary), grids, images, cubes, color
palette tables (also known as CPT), PostScript documents, and text tables (ASCII,
usually a mix of data and free-form text).  In addition, we
provide two data objects to facilitate the passing of simple user arrays
(one or more equal-length data columns of any data type, e.g., double,
char) and 2-D or 3-D user matrices (of any data type and column/row
organization). We refer to these data types as GMT *resources*.
There are many attributes for each of these resources and therefore we
use a top-level structure for each object to keep them all within one
container. These containers are given or returned by GMT API
functions using opaque pointers (``void *``). Below we provide a brief
overview of these containers, listing only the most critical members.
For complete details, see Appendix A.  We will later present how they are used when
importing or exporting them to or from files, memory locations, or
streams. The first six are the standard GMT objects, while the latter
two are special data containers to facilitate the passing of user
data in and out of GMT modules. These resources are defined in the include
file ``gmt_resources.h``; please consult this file to ensure correctness
in case the documentation is not up-to-date.  Note than in all instances
the fundamental data variable is called "data".

Data tables
~~~~~~~~~~~

Much data processed in GMT come in the form of ASCII, netCDF, or
native binary data tables. These may have any number of header records
(ASCII files only) and perhaps segment headers that separate groups of points
or lines and polygons. GMT programs will read
one or more such tables when importing data. However, to avoid memory
duplication or data limitations some programs may prefer to read such records one
at the time. The GMT API has functions that let you read your data
record-by-record by presenting a *virtual* data set that combines all the
data tables specified as input. This simplifies record processing
considerably.  Programs reading an entire data set will encounter several
structures: A data set (``struct`` :ref:`GMT_DATASET <struct-dataset>`) may contain any number of
tables (``struct`` :ref:`GMT_DATATABLE <struct-datatable>`), each with any number of segments
(``struct`` :ref:`GMT_DATASEGMENT <struct-datasegment>`), each segment with any number of
records, and each record with any number of (fixed) columns. Thus, the arguments
to GMT API functions that handle such data sets expect a struct :ref:`GMT_DATASET <struct-dataset>`.
All segments are expected to have the same number of columns.

.. _struct-dataset2:

.. code-block:: c

   struct GMT_DATASET {	/* Single container for an array of GMT tables (files) */
       uint64_t  n_tables;     /* The total number of tables contained */
       uint64_t  n_columns;    /* The number of data columns */
       uint64_t  n_segments;   /* The total number of segments across all tables */
       uint64_t  n_records;    /* The total number of data records across all tables */
       double   *min;         /* Minimum coordinate for each column */
       double   *max;         /* Maximum coordinate for each column */
       struct GMT_DATATABLE **table;    /* Pointer to array of tables */
   };

The top-level dataset structure for pure data tables contains the table structure, as defined below:

.. _struct-datatable2:

.. code-block:: c

   struct GMT_DATATABLE {  /* Single container for an array of data segments */
       unsigned int n_headers;    /* Number of table header records (0 if no header) */
       uint64_t     n_columns;    /* Number of columns (fields) in each record */
       uint64_t     n_segments;   /* Number of segments in the array */
       uint64_t     n_records;    /* Total number of data records across all segments */
       double      *min;          /* Minimum coordinate for each column */
       double      *max;          /* Maximum coordinate for each column */
       char       **header;       /* Array with all table header records, if any) */
       struct GMT_DATASEGMENT **segment; /* Pointer to array of segments */
   };

Finally, the table structure depends on a structure for individual data segments:

.. _struct-datasegment2:

.. code-block:: c

   struct GMT_DATASEGMENT {       /* For holding segment lines in memory */
       uint64_t n_rows;           /* Number of points in this segment */
       uint64_t n_columns;        /* Number of fields in each record (>= 2) */
       double  *min;              /* Minimum coordinate for each column */
       double  *max;              /* Maximum coordinate for each column */
       double **data;             /* Data x,y, and possibly other columns */
       char  **text;              /* trailing text strings beyond the data */
       char    *label;            /* Label string (if applicable) */
       char    *header;           /* Segment header (if applicable) */
    };

Data sets may have different geometries, such as representing a set of points,
one or more lines, or closed polygons.

GMT grids
~~~~~~~~~

GMT grids are used to represent equidistant and organized 2-D
surfaces. These can be processed or plotted as contour maps, color images, or
perspective surfaces. Because the native GMT grid is simply a 1-D
float array with metadata kept in a separate ``struct`` :ref:`GMT_GRID_HEADER <struct-gridheader>` header, we pass
this information via a ``struct`` :ref:`GMT_GRID <struct-grid>`, which is a container that
holds both items. Thus, the arguments to GMT API functions that handle
GMT grids expect this type of variable.

.. _struct-grid2:

.. code-block:: c

   struct GMT_GRID {                        /* A GMT float grid and header in one container */
       struct GMT_GRID_HEADER *header;      /* The full GMT header for the grid */
       float                  *data;        /* Pointer to the float grid array */
   };

The top-level grid structure, holding both header and data array, depends on the grid header structure:

.. code-block:: c

   struct GMT_GRID_HEADER {
       uint32_t n_columns;                     /* Number of columns */
       uint32_t n_rows;                        /* Number of rows */
       uint32_t registration;                  /* GMT_GRID_NODE_REG (0) for node grids,
						  GMT_GRID_PIXEL_REG (1) for pixel grids */
       double wesn[4];                         /* Min/max x and y coordinates */
       double z_min;                           /* Minimum z value */
       double z_max;                           /* Maximum z value */
       double inc[2];                          /* The x and y increments */
       double z_scale_factor;                  /* Grid values must be multiplied by this factor */
       double z_add_offset;                    /* After scaling, add this */
       char   x_units[GMT_GRID_UNIT_LEN80];    /* Units in x-direction */
       char   y_units[GMT_GRID_UNIT_LEN80];    /* Units in y-direction */
       char   z_units[GMT_GRID_UNIT_LEN80];    /* Grid value units */
       char   title[GMT_GRID_TITLE_LEN80];     /* Name of data set */
       char   command[GMT_GRID_COMMAND_LEN320];/* Name of generating command */
       char   remark[GMT_GRID_REMARK_LEN160];  /* Comments regarding this data set */
   };

   The basic grid header holds the metadata written to grid files.

GMT images
~~~~~~~~~~

GMT images are used to represent bit-mapped images typically obtained
via the GDAL bridge. These can be reprojected internally, such as when
used in :doc:`grdimage`. Since images and grids share the concept of a header,
we use the same header structure for grids as for images; however, some
additional metadata attributes are also needed. Finally, the image
itself may be of any data type and have more than one band (channel).
Both image and header information are passed via a ``struct`` :ref:`GMT_IMAGE <struct-image>`,
which is a container that holds both items. Thus, the arguments to
GMT API functions that handle GMT images expect this type of
variable. Unlike the other objects, writing images has only partial
support via :doc:`grdimage` [3]_.
For the full definition, see :ref:`GMT_IMAGE <struct-image>`.

.. _struct-image2:

.. code-block:: c

  struct GMT_IMAGE {     /* A GMT char image, header, and colormap in one container */
      enum GMT_enum_type      type;             /* Data type, e.g. GMT_FLOAT */
      int                    *colormap;         /* Array with color lookup values */
      int                     n_indexed_colors; /* Number of colors in a color-mapped image */
      struct GMT_GRID_HEADER *header;           /* Pointer to full GMT header for the image */
      unsigned char          *data;             /* Pointer to actual image */
  };


GMT cubes
~~~~~~~~~

GMT cubes are used to represent 3-D grids where all the horizontal layers
are represented by a single grid header.  Thus, all nodes along the third
dimension are coregistered and in the horizontal plane they are, like all
GMT grids, equidistant.  However, the spacing in the third dimension (which
is typically depth or time) does not have to be equidistant.  At this moment,
only :doc:`greenspline` and :doc:`grdinterpolate` can produce 3-D cubes while
the latter can also read them (other grid modules can read individual layers
of a cube as a single grid).
We use the same header structure as for grids. However, some
additional metadata attributes are needed for the third dimension.
Both the cube and header information are passed via a ``struct`` :ref:`GMT_CUBE <struct-cube>`,
which is a container that holds all items. Thus, the arguments to
GMT API functions that handle GMT cubes expect this type of
variable.
For the full definition, see :ref:`GMT_CUBE <struct-cube>`.

.. _struct-cube2:

.. code-block:: c

  struct GMT_CUBE {     /* A GMT 3-D cube in one container */
       struct GMT_GRID_HEADER *header;      /* The full GMT header for the grid */
       float                  *data;        /* Pointer to the float 3-D array */
  };

Color palette tables (CPT)
~~~~~~~~~~~~~~~~~~~~~~~~~~

The color palette table files, or just CPTs, contain colors and
patterns used for plotting data such as surfaces (i.e., GMT grids) or
symbols, lines and polygons (i.e., GMT tables). GMT programs will
generally read in a color palette table, make it the current palette, do
the plotting, and destroy the table when done. The information is
accessed via a pointer to ``struct`` :ref:`GMT_PALETTE <struct-palette>`. Thus, the arguments
to GMT API functions that handle palettes expect this type of
variable. It is not expected that users will wish to manipulate the CPT
directly, but rather use this mechanism to hold them in memory and
pass as arguments to GMT modules.  Developers are unlikely to actually
manipulate the contents of CPT structures but if needed then
the full definition can be found in :ref:`GMT_PALETTE <struct-palette>`.

.. _struct-palette2:

.. code-block:: c

   struct GMT_PALETTE {	/* Holds color-related parameters for look-up */
       unsigned int          n_headers;     /* Number of CPT header records (0 if no header) */
       unsigned int          n_colors;      /* Number of colors in the data array */
       unsigned int          mode;          /* Flags controlling use of BFN colors */
       struct GMT_LUT       *data;          /* CPT lookup data with color information */
       struct GMT_BFN        bfn[3];        /* Structures with back/fore/nan fills */
       char                **header;        /* Array with all CPT header records, if any) */
   };

PostScript document
~~~~~~~~~~~~~~~~~~~

Normally, GMT modules producing PostScript will write to standard output
or a designated file.  Alternatively, you can tell the API to write to a
memory buffer instead and then receive a structure with the final
plot (or partial plot) represented as a long text string.
The full structure definition can be found in :ref:`GMT_POSTSCRIPT <struct-postscript>`.

.. _struct-postscript2:

.. code-block:: c

   struct GMT_POSTSCRIPT {	/* Single container for a chunk of PostScript text */
       unsigned int n_headers;          /* Number of PostScript header records (0 if no header) */
       size_t n_bytes;                  /* Length of data array so far */
       unsigned int mode;               /* Bit-flag for header (1) and trailer (2) */
       char *data;                      /* Pointer to actual PostScript text */
       char **header;                   /* Array with all PostScript header records, if any) */
   };

User data matrices
~~~~~~~~~~~~~~~~~~

Users may write programs that need to call GMT modules but may keep their data in separate
2-D arrays that the allocate and maintain independent of GMT.
For instance, a program may have built an integer 2-D matrix in memory and wish to
use that as the input grid to the ``grdfilter`` module, which
normally expects a ``struct`` :ref:`GMT_GRID <struct-grid>` with floating point data via an actual or virtual
file. To handle this case we create a ``struct`` :ref:`GMT_MATRIX <struct-matrix>` container (see :ref:`Create empty resources <sec-create>`),
assign the appropriate union pointer to your data matrix and provide information on dimensions
and data type. We then open this container as a virtual file and pass its filename to any module.
The full structure definition can be found in :ref:`GMT_MATRIX <struct-matrix>`.

.. _struct-matrix2:

.. code-block:: c

   struct GMT_MATRIX {	/* Single container to hold a user matrix */
      uint64_t             n_rows;        /* Number of rows in the matrix */
      uint64_t             n_columns;     /* Number of columns in the matrix */
      uint64_t             n_layers;      /* Number of layers in a 3-D matrix */
      enum GMT_enum_fmt    shape;         /* 0 = C (rows) and 1 = Fortran (cols) */
      enum GMT_enum_reg    registration;  /* 0 for gridline and 1 for pixel registration  */
      size_t               dim;           /* Allocated length of longest C or Fortran dim */
      size_t               size;          /* Byte length of data */
      enum GMT_enum_type   type;          /* Data type, e.g. GMT_FLOAT */
      double               range[6];      /* Contains xmin/xmax/ymin/ymax[/zmin/zmax] */
      union GMT_UNIVECTOR  data;          /* Pointer to actual matrix of the chosen type */
      char               **text;          /* Pointer to optional array of strings [NULL] */
   };

The ``enum`` types referenced in :ref:`GMT_VECTOR <struct-vector>` and
Table :ref:`GMT_MATRIX <struct-matrix>` and summarized in Table :ref:`types <tbl-types>`.

User data columns
~~~~~~~~~~~~~~~~~

Likewise, programs may instead be manipulating a set of custom column vectors.
For instance, the user's program may have allocated and populated
three column arrays of type float and wishes to use these as the input
source to the ``surface`` module, which normally expects double
precision triplets via a ``struct`` :ref:`GMT_DATASET <struct-dataset>` read from an actual or virtual file
Simply create a new :ref:`GMT_VECTOR <struct-vector>` container
(see section :ref:`Create empty resources <sec-create>`) and assign the union array pointers (see
:ref:`univector <struct-univector>`) to your data columns and provide the required
information on length, data types, and optionally range. Again, once we open this data
as a virtual file we can pass its filename to any module expecting such data.
The full structure definition can be found in :ref:`GMT_VECTOR <struct-vector>`.

.. _struct-vector2:

.. code-block:: c

  struct GMT_VECTOR {	/* Single container to hold user vectors */
      uint64_t             n_columns;     /* Number of vectors */
      uint64_t             n_rows;        /* Number of rows in each vector */
      enum GMT_enum_reg    registration;  /* 0 for gridline and 1 for pixel registration */
      enum GMT_enum_type  *type;          /* Array with data type for each vector */
      union GMT_UNIVECTOR *data;          /* Array with unions for each column */
      double               range[2];      /* The min and max limits on t-range (or 0,0) */
      char               **text;          /* Pointer to optional array of strings [NULL] */
  };

Data record
~~~~~~~~~~~

For record-by-record i/o we use the GMT_RECORD structure.

.. _struct-record:

.. code-block:: c

   struct GMT_RECORD {	/* Single container for an array of GMT tables (files) */
       double  *data;   /* Pointer to array of double-precision numbers [NULL] */
       char  *text;     /* Pointer to the trailing string [NULL] */
   };


.. _chapter-overview:

Overview of the GMT C Application Program Interface
===================================================

Users who wish to create their own GMT application based on the API
must make sure their program goes through the steps below. The details for
each step will be revealed in the following chapter. We have kept the
API simple: In addition to the GMT modules, there are only 57 public
functions to become familiar with, but most applications will only use a
very small subset of this selection. Functions either return an integer error
code (when things go wrong; otherwise it is set to ``GMT_NOERROR (0)``), or they
return a void pointer to a GMT resource (or NULL if things go wrong).
In either case, the API will report what the error is. The layout here
assumes you wish to use virtual files as input sources (i.e., data you already
have in memory); if the data must be
read from actual data files then things simplify considerably.

To keep things as simple as possible we will assume you are writing an
application that will read in table data, call a module using the data in
memory as input, and then save the output from the module back into
another memory location.  No actual processing of the data or further
calculation will be done here (so a bit of a boring program, but the
point is to develop something short we can test).  Also, to keep the code
short we completely ignore
the return codes of the modules for now.  We will call our program
:ref:`example1.c <example-code1>`.  Here are the steps:

#. Initialize a new GMT session with GMT_Create_Session_, which
   allocates a hidden GMT API control structure and returns an opaque
   pointer to it. This pointer is a *required* argument to all subsequent
   GMT API function calls within the session.

#. Read a data set (or grid, etc.) into memory with GMT_Read_Data_,
   which, depending on data type, returns one of the data structures
   discussed earlier.

#. Associate your data with a virtual file using GMT_Open_VirtualFile_.
   This steps returns a special filename that you can use to tell a module where
   to read its input.  No actual file is created.

#. Open a new virtual file to hold the output using GMT_Open_VirtualFile_.
   This step also returns a special filename for the module to send its output.

#. Prepare required arguments (including the two virtual file names) and
   call the GMT module you wish to use via GMT_Call_Module.

#. Obtain the desired output object via GMT_Read_VirtualFile_, which
   returns a data structure of requested type.

#. Close the virtual files you have been using with GMT_Close_VirtualFile_.

#. We terminate the GMT session by calling GMT_Destroy_Session_.

Example code
------------

For the example code to run you must have Internet access. Compile and run
this program:

.. _example-code1:

.. code-block:: c

  #include "gmt.h"
  int main (int argc, char *argv[]) {
      void *API;        		/* The API control structure */
      struct GMT_DATASET *D = NULL;     /* Structure to hold input dataset */
      struct GMT_GRID *G = NULL;        /* Structure to hold output grid */
      char input[GMT_VF_LEN] = {""};    /* String to hold virtual input filename */
      char output[GMT_VF_LEN] = {""};   /* String to hold virtual output filename */
      char args[128] = {""};    	/* String to hold module command arguments */

      /* Initialize the GMT session */
      API = GMT_Create_Session ("test", 2U, 0, NULL);
      /* Read in our data table to memory */
      D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_PLP, GMT_READ_NORMAL, NULL,
          "@Table_5_11.txt", NULL);
      /* Associate our data table with a virtual file */
      GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_IN, D, input);
      /* Create a virtual file to hold the resulting grid */
      GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, output);
      /* Prepare the module arguments */
      sprintf (args, "-R0/7/0/7 -I0.2 -D1 -St0.3 %s -G%s", input, output);
      /* Call the greenspline module */
      GMT_Call_Module (API, "greenspline", GMT_MODULE_CMD, args);
      /* Obtain the grid from the virtual file */
      G = GMT_Read_VirtualFile (API, output);
      /* Close the virtual files */
      GMT_Close_VirtualFile (API, input);
      GMT_Close_VirtualFile (API, output);
      /* Write the grid to file */
      GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL,
          "junk.nc", G);
      /* Destroy the GMT session */
      GMT_Destroy_Session (API);
  };

Compilation
-----------

To compile this program (we assume it is called example1.c), we use the
gmt-config script to determine the correct compile and link flags and then run
gcc:

.. _example-comp:

.. code-block:: bash

   inc=`gmt-config --cflags`
   lib=`gmt-config --libs`
   gcc example1.c $inc $lib -o example1
   ./example1

This obviously assumes you have already installed GMT and that it is in your path.
If you run example1 it will take a moment (this is mostly due to the gridding
performed by :doc:`greenspline`) and then it stops.  You should find the resulting
grid junk.nc in the current directory.  Plot it to see if it makes sense, e.g.

.. _example-view:

.. code-block:: bash

   gmt grdimage junk.nc > junk.ps

If you intend to write applications that take any number of data files
via the command line then there will be more book-keeping to deal with,
and we will discuss those steps later.
Likewise, if you need to process a file record-by-record then more lines
of code will be required.

Plugins
-------

Developers who wish to make custom plugin libraries that are compatible
with GMT should examine the fully functioning examples of more involved
code, available from the repository gmt-custom, obtainable via

.. code-block:: bash

   git clone https://github.com/GenericMappingTools/gmt-custom.git


List of API functions
---------------------

The following is an alphabetical listing of all the public API functions in GMT. Click on
any of them to see the full syntax of each function.

The C/C++ API is deliberately kept small to make it easy to use.

.. _tbl-API:

    +--------------------------+-------------------------------------------------------+
    | constant                 | description                                           |
    +==========================+=======================================================+
    | GMT_Alloc_Segment_       | Allocate data segments                                |
    +--------------------------+-------------------------------------------------------+
    | GMT_Append_Option_       | Append new option structure to linked list            |
    +--------------------------+-------------------------------------------------------+
    | GMT_Begin_IO_            | Enable record-by-record i/o                           |
    +--------------------------+-------------------------------------------------------+
    | GMT_Call_Module_         | Call any of the GMT modules                           |
    +--------------------------+-------------------------------------------------------+
    | GMT_Convert_Data_        | Convert between compatible data types                 |
    +--------------------------+-------------------------------------------------------+
    | GMT_Close_VirtualFile_   | Close a virtual file                                  |
    +--------------------------+-------------------------------------------------------+
    | GMT_Create_Args_         | Convert linked list of options to text array          |
    +--------------------------+-------------------------------------------------------+
    | GMT_Create_Cmd_          | Convert linked list of options to command line        |
    +--------------------------+-------------------------------------------------------+
    | GMT_Create_Data_         | Create an empty data resource                         |
    +--------------------------+-------------------------------------------------------+
    | GMT_Create_Options_      | Convert command line options to linked list           |
    +--------------------------+-------------------------------------------------------+
    | GMT_Create_Session_      | Initialize a new GMT session                          |
    +--------------------------+-------------------------------------------------------+
    | GMT_Delete_Option_       | Delete an option structure from the linked list       |
    +--------------------------+-------------------------------------------------------+
    | GMT_Destroy_Args_        | Delete text array of arguments                        |
    +--------------------------+-------------------------------------------------------+
    | GMT_Destroy_Cmd_         | Delete text command of arguments                      |
    +--------------------------+-------------------------------------------------------+
    | GMT_Destroy_Data_        | Delete a data resource                                |
    +--------------------------+-------------------------------------------------------+
    | GMT_Destroy_Group_       | Delete a group of data resources                      |
    +--------------------------+-------------------------------------------------------+
    | GMT_Destroy_Options_     | Delete the linked list of option structures           |
    +--------------------------+-------------------------------------------------------+
    | GMT_Destroy_Session_     | Terminate a GMT session                               |
    +--------------------------+-------------------------------------------------------+
    | GMT_Duplicate_Data_      | Make an identical copy of a data resources            |
    +--------------------------+-------------------------------------------------------+
    | GMT_Encode_Options_      | Encode option arguments for external interfaces       |
    +--------------------------+-------------------------------------------------------+
    | GMT_Error_Message_       | Return character pointer to last API error message    |
    +--------------------------+-------------------------------------------------------+
    | GMT_Expand_Option_       | Expand option with explicit memory references         |
    +--------------------------+-------------------------------------------------------+
    | GMT_End_IO_              | Disable further record-by-record i/o                  |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_                 | Take the Fast Fourier Transform of data object        |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_1D_              | Take the Fast Fourier Transform of 1-D float data     |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_2D_              | Take the Fast Fourier Transform of 2-D float data     |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_Create_          | Initialize the FFT machinery                          |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_Destroy_         | Terminate the FFT machinery                           |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_Option_          | Explain the FFT options and modifiers                 |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_Parse_           | Parse argument with FFT options and modifiers         |
    +--------------------------+-------------------------------------------------------+
    | GMT_FFT_Wavenumber_      | Return wavenumber given data index                    |
    +--------------------------+-------------------------------------------------------+
    | GMT_Find_Option_         | Find an option in the linked list                     |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Common_          | Determine if a GMT common option was set              |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Coord_           | Create a coordinate array                             |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Default_         | Obtain one of the API or GMT default settings         |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Enum_            | Obtain one of the API enum constants                  |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_FilePath_        | Verify input file exist and replace with full path    |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Index_           | Convert row, col into a grid or image 1-D index       |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Index3_          | Convert row, col, layer into a cube 1-D index         |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Info_            | Obtain meta data (range, dimension), ... from object  |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Matrix_          | Obtain pointer to user matrix from container          |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Pixel_           | Get grid or image node                                |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Record_          | Import a single data record                           |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Row_             | Import a single grid row                              |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Status_          | Check status of record-by-record i/o                  |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Strings_         | Obtain pointer to user strings from matrix or vector  |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Values_          | Convert string into coordinates or dimensions         |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Vector_          | Obtain pointer to user vector from container          |
    +--------------------------+-------------------------------------------------------+
    | GMT_Get_Version_         | Return the current lib version as a float             |
    +--------------------------+-------------------------------------------------------+
    | GMT_Init_IO_             | Initialize i/o given registered resources             |
    +--------------------------+-------------------------------------------------------+
    | GMT_Init_VirtualFile_    | Reset a virtual file for reuse                        |
    +--------------------------+-------------------------------------------------------+
    | GMT_Inquire_VirtualFile_ | Get family of a virtual file                          |
    +--------------------------+-------------------------------------------------------+
    | GMT_Make_Option_         | Create an option structure                            |
    +--------------------------+-------------------------------------------------------+
    | GMT_Message_             | Issue a message, optionally with time stamp           |
    +--------------------------+-------------------------------------------------------+
    | GMT_Open_VirtualFile_    | Select memory location as input or output for module  |
    +--------------------------+-------------------------------------------------------+
    | GMT_Option_              | Explain one or more GMT common options                |
    +--------------------------+-------------------------------------------------------+
    | GMT_Parse_Common_        | Parse the GMT common options                          |
    +--------------------------+-------------------------------------------------------+
    | GMT_Put_Levels_          | Put user level coordinates into cube container        |
    +--------------------------+-------------------------------------------------------+
    | GMT_Put_Matrix_          | Put user matrix into container                        |
    +--------------------------+-------------------------------------------------------+
    | GMT_Put_Record_          | Export a data record                                  |
    +--------------------------+-------------------------------------------------------+
    | GMT_Put_Row_             | Export a grid row                                     |
    +--------------------------+-------------------------------------------------------+
    | GMT_Put_Strings_         | Put user strings into vector or matrix container      |
    +--------------------------+-------------------------------------------------------+
    | GMT_Put_Vector_          | Put user vector into container                        |
    +--------------------------+-------------------------------------------------------+
    | GMT_Read_Data_           | Import a data resource or file                        |
    +--------------------------+-------------------------------------------------------+
    | GMT_Read_Group_          | Import a group of data resources or files             |
    +--------------------------+-------------------------------------------------------+
    | GMT_Read_VirtualFile_    | Access the output from a module via memory            |
    +--------------------------+-------------------------------------------------------+
    | GMT_Register_IO_         | Register a resources for i/o                          |
    +--------------------------+-------------------------------------------------------+
    | GMT_Report_              | Issue a message contingent upon verbosity level       |
    +--------------------------+-------------------------------------------------------+
    | GMT_Set_Default_         | Set one of the API or GMT default settings            |
    +--------------------------+-------------------------------------------------------+
    | GMT_Set_Comment_         | Assign a comment to a data resource                   |
    +--------------------------+-------------------------------------------------------+
    | GMT_Set_Columns_         | Specify how many columns to use for rec-by-rec i/o    |
    +--------------------------+-------------------------------------------------------+
    | GMT_Set_Geometry_        | Specify data geometry for rec-by-rec i/o              |
    +--------------------------+-------------------------------------------------------+
    | GMT_Set_Index_           | Convert row, col into a grid or image index           |
    +--------------------------+-------------------------------------------------------+
    | GMT_Update_Option_       | Modify an option structure                            |
    +--------------------------+-------------------------------------------------------+
    | GMT_Write_Data_          | Export a data resource                                |
    +--------------------------+-------------------------------------------------------+

    Summary of all the API functions and their purpose.

The GMT C Application Program Interface
=======================================

Initialize a new GMT session
----------------------------

Advanced programs may be calling more than one GMT session and thus
run several sessions, perhaps concurrently as different threads on
multi-core machines. We will now discuss these steps in more detail.
Throughout, we will introduce upper-case GMT C enum constants *in
lieu* of simple integer constants. These are considered part of the API
and are available for developers via the ``gmt_resources.h`` include file.

Most applications will need to initialize only a single GMT session.
This is true of all the standard GMT programs since they only call one
GMT module and then exit. Most user-developed GMT applications are
likely to only initialize one session even though they may call many
GMT modules. However, the GMT API supports any number of
simultaneous sessions should the programmer wish to take advantage of
it. This might be useful when you have access to several CPUs and want
to spread the computing load [4]_. In the following discussion we will
simplify our treatment to the use of a single session only.

To initiate the new GMT session we use

.. _GMT_Create_Session:

  ::

    void *GMT_Create_Session (const char *tag, unsigned int pad, unsigned int mode,
    	int (*print_func) (FILE *, const char *));

and you will typically call it like this:

  ::

    void *API = NULL;	/* Opaque pointer to GMT controls */
    API = GMT_Create_Session ("Session name", 2, 0, NULL);

where ``API`` is an opaque pointer to the hidden GMT API control
structure. You will need to pass this pointer to *all* subsequent
GMT API functions; this is how essential internal information is
passed around. The key task of this initialization is to
set up the GMT machinery and internal variables used for map
projections, plotting, i/o, etc. The initialization also allocates space
for internal structures used to keep track of data. The ``pad`` argument
specifies how many rows and columns should be used as padding for grids and
images so that boundary conditions can be applied. GMT uses 2 and we strongly
recommend that you use that value. In particular, if you choose 0 or 1 there may be certain
GMT modules that will be unable to do their work properly as they count on those
boundary rows and columns in the grids.  Note that this setting has no effect
on what is written to a grid file; the padding is an internal feature.
The ``mode`` argument is only used for external APIs that need
to communicate their special needs during the session creation.  This integer argument
is a sum of bit flags and the various bits control the following settings:

#. Bit 1 (1 or GMT_SESSION_NOEXIT): If set, then GMT will not call the system exit function when a
   serious problem has been detected but instead will simply return control
   to the calling environment.  For instance, this is required by the GMT/MATLAB toolbox
   since calling exit would also exit MATLAB itself.  Unless your environment
   has this feature you should leave this bit alone.
#. Bit 2 (2 or GMT_SESSION_EXTERNAL): If set, then it means we are calling the GMT API from an external
   API, such as MATLAB, Octave, or Python.  Normal C/C++ programs should
   leave this bit alone.  Its effect is to enable two additional modules
   for reading and writing GMT resources from these environments (those modules
   would not make any sense in a Unix command-line environment).
#. Bit 3 (4 or GMT_SESSION_COLMAJOR): If set, then it means the external API uses a column-major format for
   matrices (e.g., MATLAB, Fortran).  If not set we default to row-major
   format (C/C++, Python, etc.).
#. Big 4 (8 or GMT_SESSION_LOGERRORS): If set, we redirect all error messages to a log file based on the
   session name (we append ".log").
#. Bit 5 (16 or GMT_SESSION_RUNMODE): If set, the we enable GMT's modern run-mode (where -O -K are
   not allowed and PostScript is written to hidden temp file).  Default
   is the GMT classic run-mode.
#. Bit 6 (32 or GMT_SESSION_NOHISTORY): If set, the we disable GMT's command shorthand via gmt.history files.
   The default is to allow this communication between GMT modules.

The ``print_func`` argument is a pointer to a function that is used to print
messages from GMT via GMT_Message_ or GMT_Report_ from external environments that cannot use the
standard printf function (this is the case for the GMT/MATLAB toolbox, for instance).
For all other uses you should simply pass NULL for this argument.  You can also access
the last cached error message by calling GMT_Error_Message_ which returns a pointer to
the internal character buffer with that message.  Pass NULL and set the mode bit if you
want writing to a log file instead.
Should something go wrong during the API initialization then ``API`` will be returned as ``NULL``.
Finally, GMT_Create_Session_ will examine the environmental parameter TMPDIR (TEMP on Windows)
to set the GMT temporary directory [/tmp on Unix, current directory on Windows].

Below is a bare-bones minimalistic GMT program hello.c that initializes and destroys
a GMT session:

.. _example-code2:

.. code-block:: c

  #include "gmt.h"
  int main (int argc, char *argv[]) {
  	void *API;	/* The API control structure */
  	/* Initialize the GMT session */
  	API = GMT_Create_Session ("test", 2U, 0, NULL);
	/* And now for something original: */
	GMT_Message (API, GMT_TIME_NONE, "hello, world\n");
  	/* Destroy the GMT session */
  	GMT_Destroy_Session (API);
  };

While not super-exiting, this code demonstrates the two essential API calls
required to initiate and later terminate a GMT session.  In between we do what
all basic programs are supposed to do: print "Hello, world".  The user is of course
allowed to do whatever custom processing before the GMT session is created
and can do all sorts of stuff after the GMT session is destroyed, as long as
no GMT functions or resources are accessed.  It may be convenient to isolate
the GMT-specific processing from the custom part of the program and only
maintain an active GMT session when needed.

Get full path to local or remote files
--------------------------------------

If given a filename, GMT will look in several directories to find the given
input file.  However, GMT can also look for files remotely, either via the
remote file mechanism or URLs.  When you have a remote file (@filename) you
may wish to have GMT automatically download the file and provide you with the
local path.  This is a job for GMT_Get_FilePath_, whose prototype is

.. _GMT_Get_FilePath:

  ::

    int GMT_Get_FilePath (void *API, unsigned int family, unsigned int direction,
      unsigned int mode, char **ptr);

where :ref:`family <tbl-family>` and ``direction`` set the data file type and whether it is
for input or output, ``mode`` modifies the behavior of the function, and
``*ptr`` is a pointer to a character string with the filename in question.  Normally,
we only look for local files (GMT_FILE_LOCAL [0]), but if ``mode`` contains
the bit flag GMT_FILE_REMOTE [1] we will try to download any remote files given
to the function.  By default, we will replace the filename with the full
path.  Add the bit flag GMT_FILE_CHECK [2] to only check for the files and return
error codes but leave ``*ptr`` alone.


Register input or output resources
----------------------------------

When using the standard GMT programs, it is common to specify input files on the
command line or via special program options (e.g.,
**-I**\ *intensity.nc*). The outputs of the programs are either written
to standard output (which you may redirect to files or pipes into other
programs) or to files specified by specific program options (e.g.,
**-G**\ *output.nc*). Alternatively, the GMT API allows you to specify
input (and output) to be associated with open file handles or virtual files.
We will examine this more closely below. Registering a
resource is a required step before attempting to import or export data
that *do not* come from files or standard input/output.

.. _sec-res_init:

Resource initialization
~~~~~~~~~~~~~~~~~~~~~~~

All GMT programs dealing with input or output files given on the
command line, and perhaps defaulting to the standard input or output
streams if no files are given, must call the i/o initializer function
GMT_Init_IO_ once for each direction required (i.e., input and output
separately). For input it determines how many input sources have already
been registered. If none has been registered then it scans the program
arguments for any filenames given on the command line and register these
input resources. Finally, if we still have found no input sources we
assign the standard input stream as the single input source. For output
it is similar: If no single destination has been registered we specify
the standard output stream as the output destination. Only one main
output destination is allowed to be active when a module writes data
(some modules also write additional output via program-specific
options). The prototype for this function is

.. _GMT_Init_IO:

  ::

    int GMT_Init_IO (void *API, unsigned int family, unsigned int geometry,
    	unsigned int direction, unsigned int mode, unsigned int n_args, void *args);

where :ref:`family <tbl-family>` specifies what kind of resource is to be registered,
:ref:`geometry <tbl-geometry>` specifies the geometry of the data, ``direction`` is either
``GMT_IN`` or ``GMT_OUT``, and ``mode`` is a bit flag that determines
what we do if no resources have been registered. The choices are

    **GMT_ADD_FILES_IF_NONE** (1) means "add command line (option)
    files if none have been registered already".

    **GMT_ADD_FILES_ALWAYS** (2) means "always add any command line files".

    **GMT_ADD_STDIO_IF_NONE** (4) means "add std\* if no other
    input/output have been specified".

    **GMT_ADD_DEFAULT** (6) means "always add any command line files first, and then
    add std\* if no other input/output were specified".

    **GMT_ADD_STDIO_ALWAYS** (8) means "always add std\* even if
    resources have been registered".

    **GMT_ADD_EXISTING** (16) means "only use already registered resources".

The standard behavior is ``GMT_ADD_DEFAULT`` (6). Next, ``n_args`` is 0
if ``args`` is the head of a linked list of options (further discussed
in :ref:`Prepare modules opts <sec-func>`); otherwise ``args`` is an array of ``n_args``
strings (i.e., the int argc, char \*argv[] model)

Many programs will register an export location where results of a GMT function (say, a filtered grid)
should be returned, but may then wish to use that variable as an *input* resource in a subsequent module
call. This is accomplished by re-registering the resource as an *input* source, thereby changing the
*direction* of the data set. The function returns 1 if there is an error; otherwise it returns 0. |ex_resource_init|

Resource registration
~~~~~~~~~~~~~~~~~~~~~

Should your program need to add additional sources (or a destination) to the list of items
to be considered you will need to register them manually.  This is considered a low-level
activity and most applications are unlikely to have to resort to this step.  We document
it here in case your situation calls for such action.
Registration involves a direct or indirect call to

.. _GMT_Register_IO:

  ::

    int GMT_Register_IO (void *API, unsigned int family, unsigned int method,
    	unsigned int geometry, unsigned int direction, double wesn[], void *ptr);

where :ref:`family <tbl-family>` specifies what kind of resource is to be registered,
:ref:`method <tbl-methods>` specifies
how we to access this resource (see Table :ref:`methods <tbl-methods>` for recognized
methods), :ref:`geometry <tbl-geometry>` specifies the geometry of the data, ``ptr`` is the address of the
pointer to the named resource. If ``direction`` is ``GMT_OUT`` and the
``method`` is not related to a file (filename, stream, or handle), then
``ptr`` must be NULL. Note there are some limitations on when you may pass a file pointer
as the method.  Many grid file formats cannot be read via a stream (e.g., netCDF files) so in
those situations you cannot pass a file pointer [and GMT_Register_IO would have no way of knowing
this].  For grid (and image)
resources you may request to obtain a subset via the :ref:`wesn <tbl-wesn>` array; otherwise, pass NULL
(or an array with at least 4 items all set to 0) to obtain the
entire grid (or image). The ``direction`` indicates input or output and
is either ``GMT_IN`` or ``GMT_OUT``. Finally, the function returns a
unique resource ID, or ``GMT_NOTSET`` if there was an error.


.. _tbl-family:

    +-------------------+---------------------------------+
    | family            | source points to                |
    +===================+=================================+
    | GMT_IS_DATASET    | A [multi-segment] data file     |
    +-------------------+---------------------------------+
    | GMT_IS_GRID       | A grid file                     |
    +-------------------+---------------------------------+
    | GMT_IS_IMAGE      | An image                        |
    +-------------------+---------------------------------+
    | GMT_IS_CUBE       | A 3-D cube                      |
    +-------------------+---------------------------------+
    | GMT_IS_PALETTE    | A color palette table [CPT]     |
    +-------------------+---------------------------------+
    | GMT_IS_POSTSCRIPT | A GMT PostScript object         |
    +-------------------+---------------------------------+
    | GMT_IS_MATRIX     | A custom user data matrix       |
    +-------------------+---------------------------------+
    | GMT_IS_VECTOR     | A custom user data vector       |
    +-------------------+---------------------------------+
    | GMT_VIA_MATRIX    | Modifier for grids and datasets |
    +-------------------+---------------------------------+
    | GMT_VIA_VECTOR    | Modifier for grids and datasets |
    +-------------------+---------------------------------+

    GMT constants used to specify a data family.

.. _tbl-methods:

    +------------------+------------------------------------------------+
    | method           | how to read/write data                         |
    +==================+================================================+
    | GMT_IS_FILE      | Pointer to name of a file                      |
    +------------------+------------------------------------------------+
    | GMT_IS_STREAM    | Pointer to open stream (or process)            |
    +------------------+------------------------------------------------+
    | GMT_IS_FDESC     | Pointer to integer file descriptor             |
    +------------------+------------------------------------------------+
    | GMT_IS_DUPLICATE | Pointer to memory we may *duplicate* data from |
    +------------------+------------------------------------------------+
    | GMT_IS_REFERENCE | Pointer to memory we may *reference* data from |
    +------------------+------------------------------------------------+

    GMT constants used to specify how data will be read or written.

.. _tbl-geometry:

    +----------------+-----------------------------------------+
    | geometry       |  description                            |
    +================+=========================================+
    | GMT_IS_NONE    | Not a geographic feature                |
    +----------------+-----------------------------------------+
    | GMT_IS_POINT   | Multi-dimensional point data            |
    +----------------+-----------------------------------------+
    | GMT_IS_LINE    | Geographic or Cartesian line segments   |
    +----------------+-----------------------------------------+
    | GMT_IS_POLYGON | Geographic or Cartesian closed polygons |
    +----------------+-----------------------------------------+
    | GMT_IS_PLP     | Either points, lines, or polygons       |
    +----------------+-----------------------------------------+
    | GMT_IS_SURFACE | 2-D gridded surface                     |
    +----------------+-----------------------------------------+
    | GMT_IS_VOLUME  | 3-D gridded volume                      |
    +----------------+-----------------------------------------+

    GMT constants used to specify the geometry of the data object.

.. _tbl-wesn:

    +---------+----------------------------------------------+
    | index   |  description                                 |
    +=========+==============================================+
    | GMT_XLO | x_min (west) boundary of grid subset         |
    +---------+----------------------------------------------+
    | GMT_XHI | x_max (east) boundary of grid subset         |
    +---------+----------------------------------------------+
    | GMT_YLO | y_min (south) boundary of grid subset        |
    +---------+----------------------------------------------+
    | GMT_YHI | y_max (north) boundary of grid subset        |
    +---------+----------------------------------------------+
    | GMT_ZLO | z_min (bottom) boundary of 3-D matrix subset |
    +---------+----------------------------------------------+
    | GMT_ZHI | z_max (top) boundary of 3-D matrix subset    |
    +---------+----------------------------------------------+

    GMT constants used for domain array indexing.

.. _sec-create:

Create empty resources
----------------------

If your application needs to build and populate GMT resources in ways
that do not depend on external resources (files, memory locations,
etc.), or you have data read in separately and you wish to build a
GMT resource from scratch, then you can obtain an empty object by calling

.. _GMT_Create_Data:

  ::

    void *GMT_Create_Data (void *API, unsigned int family, unsigned int geometry,
         unsigned int mode, uint64_t par[], double *wesn, double *inc,
         unsigned int registration, int pad, void *data)

which returns a pointer to the allocated resource. Pass a valid :ref:`family <tbl-family>` selection.
Also pass a compatible :ref:`geometry <tbl-geometry>`. Depending on the family and your particular way of
representing dimensions you may pass the additional parameters in one of
two ways:

#. Actual integer dimensions of items needed (which depends on the ``family``).

#. Physical distances and increments of each dimension.

For the first case you should pass both ``wesn`` and ``inc`` as NULL (or as arrays with elements all set to 0),
and pass the ``par`` array with contents as indicated below:

  **GMT_IS_GRID**.
    An empty :ref:`GMT_GRID <struct-grid>` structure with a header is allocated; the data
    array is NULL.  Use ``registration`` to choose either gridline (``GMT_GRID_PIXEL_REG``) or pixel
    (``GMT_GRID_NODE_REG``) registration.  The domain can be prescribed on one of two ways:
    (1) The ``par`` argument is NULL. Then ``wesn`` and ``inc`` can also be NULL but only if the common GMT options
    **-R** and **-I** have been set because they are required to get the necessary info. If they
    were not set, then ``wesn`` and ``inc`` must in fact be transmitted.  If ``wesn`` and ``inc``
    are set (directly or indirectly) then ``par`` is ignored, even if not NULL.
    (2) The ``par`` argument is not NULL but both ``wesn`` and ``inc`` are NULL.
    Now, ``par[0]`` must have the number of columns and ``par[1]`` must have the number of rows in the grid.  Internally,
    ``inc`` will be set to 1/1 and ``wesn`` will be set to 0/n_columns/0/n_rows. As an option, add ``GMT_GRID_XY`` to ``mode``
    and we also allocate the grids's *x* and *y* coordinate vectors.

  **GMT_IS_IMAGE**.
    Same procedure as for **GMT_IS_GRID** but we return an empty :ref:`GMT_IMAGE <struct-image>` object.  In either
    way of specification you may use ``par[2]`` to pass the number of image bands [1].

  **GMT_IS_CUBE**.
    Same procedure as for **GMT_IS_GRID** but both ``wesn``, ``inc`` and ``par`` have one extra
    dimension for the depth or time axis.  For non-equidistant layers you need to use
    ``par[2]`` to sets the number of layers and use ``inc[2] = 0``, otherwise ``wesn`` and ``inc`` can set it all.

  **GMT_IS_DATASET**.
    We allocate an empty :ref:`GMT_DATASET <struct-dataset>` structure consisting of ``par[0]`` tables,
    each with ``par[1]`` segments, each with ``par[2]`` rows, all with ``par[3]`` columns.
    The ``wesn``, ``inc``, and ``registration`` argument are ignored.  The ``data`` argument should be NULL. As an option,
    add ``GMT_WITH_STRINGS`` to ``mode`` and we also allocate the segments' *text* field.

  **GMT_IS_PALETTE**.
    We allocate an empty :ref:`GMT_PALETTE <struct-palette>` structure with ``par[0]`` palette entries.
    The ``wesn``, ``inc``, and ``registration`` arguments are ignored and should be NULL/0.  The ``data`` argument should be NULL.

  **GMT_IS_POSTSCRIPT**.
    We allocate an empty :ref:`GMT_POSTSCRIPT <struct-postscript>` structure with a text buffer of length ``par[0]``.
    Give ``par[0]`` = 0 if the PostScript string is allocated or obtained by other means.
    The ``wesn``, ``inc``, and ``registration`` arguments are ignored and should be NULL/0.  The ``data`` argument should be NULL.

  **GMT_IS_VECTOR**.
    We allocate an empty :ref:`GMT_VECTOR <struct-vector>` structure with ``par[0]`` column entries.
    The number of rows can be specified in one of two ways: (1) Set the number of rows via ``par[1]``. Then,
    ``wesn``, ``inc``, and ``registration`` arguments are ignored.
    (2) Specify ``wesn``, ``inc``, and ``registration`` and the number of rows will be computed from these
    parameters instead.  Finally, ``par[2]`` holds the data type of all vectors, if you are allocating them here.
    The ``data`` argument should be NULL.  If you have custom vectors you wish to use then
    pass ``par`` but make sure to select mode GMT_CONTAINER_ONLY so that no memory is allocated.  Furthermore,
    if you are manually setting up output containers then pass mode as GMT_IS_OUTPUT instead.
    Use GMT_Put_Vector_ to hook up your vectors.

  **GMT_IS_MATRIX**.
    We allocate an empty :ref:`GMT_MATRIX <struct-matrix>` structure. The domain can be prescribed on one of two ways:
    (1) Here, ``par[0]`` is the number of columns while ``par[1]`` has the number of rows.  Also,
    ``par[2]`` indicates the number of layers for a 3-D matrix, or pass 0, 1, or NULL for a 2-D matrix.
    Finally, ``par[3]`` holds the data type of the matrix, if you are allocating one.
    (2) Pass ``wesn``, ``inc``, ``registration`` and we compute the dimensions of the matrix.
    The ``data`` argument should be NULL.  As for vectors, to use custom data you must (for input) pass the
    mode as GMT_CONTAINER_ONLY and hook your custom matrix in via a call to GMT_Put_Matrix_.  The matrix may either
    be row- or column-oriented and this is normally determined when you created the session with GMT_Create_Session_ (see the bit 3 setting).
    However, you can pass ``pad`` = 1 (GMT_IS_ROW_FORMAT; set row major) or ``pad`` = 2 (GMT_IS_COL_FORMAT; set col major) to override the default.
    As for vectors, if this container is for output then pass mode as GMT_IS_OUTPUT instead.

Users wishing to pass their own data matrices and vectors to GMT modules will need to do so via
the **GMT_IS_MATRIX** and **GMT_IS_VECTOR** containers.  However, no module deals with such containers
directly (they either expect **GMT_IS_GRID** or **GMT_IS_DATASET**, for instance).
The solution is to specify the container type the GMT module expects but add in the special
flags **GMT_VIA_MATRIX** or **GMT_VIA*VECTOR**.  This will create the **GMT_IS_MATRIX** or
**GMT_IS_VECTOR** container the user needs to add the user data, but will also tell GMT how
they should be considered by the module.

For grids and images you may pass ``pad`` to set the padding, or -1 to
accept the prevailing GMT default. The ``mode`` determines what is actually
allocated when you have chosen grids or images. As for GMT_Read_Data_
you can pass ``GMT_CONTAINER_AND_DATA`` to initialize the header *and* allocate
space for the array; here ``data`` must be NULL. Alternatively, you can pass
``GMT_CONTAINER_ONLY`` to just initialize the grid or image header,
and later call GMT_Create_Data a second time, now passing ``GMT_DATA_ONLY``, to allocate
space for the array. In that second call you pass the pointer returned
by the first call as ``data`` and specify the family; all other
arguments should be NULL or 0. Normally, resources created by this
function are considered to be input (i.e., have a direction that is ``GMT_IN``).
The exception to this is for containers to hold results from GMT which need have a direction
set to ``GMT_OUT``.   Such empty containers are requested by passing mode = ``GMT_IS_OUTPUT``
and setting all dimension arguments to 0 or NULL.
The function returns a pointer to the
data container. In case of an error we return a NULL pointer and pass an
error code via ``API->error``. Your C code will have to include "gmt_private.h" to be able to
dereference the API pointer.

Hooking user arrays to objects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you have custom column vector or matrices and you want them to be used as
input to GMT modules, you will need to create a :ref:`GMT_VECTOR <struct-vector>` or :ref:`GMT_MATRIX <struct-matrix>` container
and hook your items to them.  Likewise, if you want to receive the output of GMT modules
into user arrays or matrices then you will need to access those data.
The following utility functions are used for these tasks:

.. _GMT_Put_Matrix:

  ::

    int GMT_Put_Matrix (void *API, struct GMT_MATRIX *M, unsigned int type, int pad, void *matrix);

where ``M`` is a :ref:`GMT_MATRIX <struct-matrix>` created by GMT_Create_Data_, the ``type`` is one of the
recognized data :ref:`types <tbl-types>`, ``pad`` indicates if the matrix has or should have padding,
and ``matrix`` is your custom matrix.  The ``pad`` entry is typically 0 (no pad present), but if you
intend the matrix to serve as grid input to a module then GMT will expect 2.  If your matrix already has
been extended by 2 extra rows and columns then pass ``pad`` = 2.
To extract a custom matrix from an output :ref:`GMT_MATRIX <struct-matrix>` you can use

.. _GMT_Get_Matrix:

  ::

    void *GMT_Get_Matrix (void *API, struct GMT_MATRIX *M);

which simply returns a pointer to the right union pointer.
For vectors the same principles apply:

.. _GMT_Put_Vector:

  ::

    int GMT_Put_Vector (void *API, struct GMT_VECTOR *V, unsigned int col,
	unsigned int type, void *vector);

where ``V`` is the :ref:`GMT_VECTOR <struct-vector>` created by GMT_Create_Data_, ``col`` is the vector
column in question, ``type`` is one of the
recognized data :ref:`types <tbl-types>` used for this vector, and ``vector`` is
a pointer to this custom vector.  In addition, ``type`` may be also **GMT_DATETIME**, in which case
we expect an array of strings with ISO datetime strings and we do the conversion to internal
GMT time and allocate a vector to hold the result in the given ``col``.
To extract a custom vector from an output :ref:`GMT_VECTOR <struct-vector>` you can use

.. _GMT_Get_Vector:

  ::

    void *GMT_Get_Vector (void *API, struct GMT_VECTOR *V, unsigned int col);

where ``col`` is the vector number you wish to obtain a pointer to.

.. _GMT_Put_Levels:

  ::

    int GMT_Put_Levels (void *API, struct GMT_CUBE *C, double *levels, uint64_t n_levels);

where ``C`` is the :ref:`GMT_CUBE <struct-cube>` created by GMT_Create_Data_, ``levels`` is an array
with the (probably) non-equidistant coordinates for the third cube dimension, and ``n_levels`` is their number.
This function is typically used when we are creating a cube whose spacing between layers is not equidistant
and hence cannot be computed internally from range and increment.

.. _GMT_Get_Version:

  ::

    void *GMT_Get_Version (void *API, unsigned int *major, unsigned int *minor, unsigned int *patch);

Returns the current lib version as a float, e.g. *6.0*, and optionally its constituints. Either one or all
of in *\ *major*, *\ *minor*, *\ *patch* args can be NULL. If they are not, one gets the corresponding
version component. The *API* pointer is actually not used in this function, so passing NULL is the best
option.

Finally, for either vectors or matrices you may optionally add a pointer to an
array of text strings, one per row.  This is done via

.. _GMT_Put_Strings:

  ::

    int GMT_Put_Strings (void *API, unsigned int family, void *X, char **array);

where ``family`` is either GMT_IS_VECTOR or GMT_IS_MATRIX, ``X`` is either a
:ref:`GMT_VECTOR <struct-vector>` or :ref:`GMT_MATRIX <struct-matrix>`, and
``array`` is the a pointer to your string array.  You may add ``GMT_IS_DUPLICATE`` to
``family`` to indicate you want the array of strings to be duplicated; the default
is to just set a pointer to ``array``.

To access the string array from an output vector or matrix container you will use

.. _GMT_Get_Strings:

  ::

    char **GMT_Get_Strings (void *API, unsigned int family, void *X);

where again ``family`` is either GMT_IS_VECTOR or GMT_IS_MATRIX and  ``X`` is either a
:ref:`GMT_VECTOR <struct-vector>` or :ref:`GMT_MATRIX <struct-matrix>`.


Manually add segments
~~~~~~~~~~~~~~~~~~~~~

If you do not know the number of rows in the segments or you expect different segments to have different
lengths then you should set the row dimension to zero in GMT_Create_Data and add the segments
manually with ``GMT_Alloc_Segment``, which allocates a new :ref:`GMT_DATASET <struct-dataset>` segment
for such multi-segment tables.

.. _GMT_Alloc_Segment:

  ::

    void *GMT_Alloc_Segment (void *API, unsigned int mode,
    	uint64_t n_rows, uint64_t n_columns, char *header, void *S);

where ``header`` is the segment's desired header (or NULL) and `mode` determines if the
segment should allocate a string array, which in this case should either be ``GMT_NO_STRINGS``
or ``GMT_WITH_STRINGS``.  If ``S`` is not NULL then we simply reallocate the lengths
of the segment; otherwise a new segment is first allocated.

There is also the option of controlling the allocation of the segment
array by setting n_rows = 0.  This would allow external arrays (double-precision only) to connect to
the S->data[col] arrays and not be freed by GMT's garbage collector.


Get information (meta data) about object
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you are creating objects in an environment where the objects are opaque pointers, then it may
be necessary to inquire about an objects dimension, range, registration, padding, etc.  We can
do this with


.. _GMT_Get_Info:

  ::

    void *_GMT_Get_Info (void *API, unsigned int family, void *data, unsigned int *geometry,
	uint64_t dim[], double *range, double *inc, unsigned int *registration, int *pad)

where ``family`` is the type of object referenced by ``data``. Depending on the type of object,
one or more of ``dim``, ``range``, ``inc``, ``registration``, and ``pad`` will be initialized,
but only if they do not point to NULL.  The function returns an error code if an invalid family
was selected.


Duplicate resources
-------------------

Often you have read or created a data resource and then need an
identical copy, presumably to make modifications to. Or, you want a copy
with the same dimensions and allocated memory, except data values should
not be duplicated. Alternatively, perhaps you just want to duplicate the
header and skip the allocation and duplication of the data entirely. These tasks
are addressed by

.. _GMT_Duplicate_Data:

  ::

    void *GMT_Duplicate_Data (void *API, unsigned int family, unsigned int mode,
    	void *data);

which returns a pointer to the allocated resource. Specify which
:ref:`family <tbl-family>` and select ``mode`` from ``GMT_DUPLICATE_DATA``,
``GMT_DUPLICATE_ALLOC``, and ``GMT_DUPLICATE_NONE``, as discussed above
(also see ``mode`` discussion above). For :ref:`GMT_GRID <struct-grid>`
you may add ``GMT_DUPLICATE_RESET`` which will ensure the duplicate grid
will have normal padding (useful when the original has non-standard padding).
For :ref:`GMT_DATASET <struct-dataset>` you can
add modifiers ``GMT_ALLOC_VERTICAL`` or ``GMT_ALLOC_HORIZONTAL`` to the ``mode`` if you
wish to put all the data into a single long table or to paste all tables
side-by-side, respectively (thus getting one wide table instead).
Additional note for :ref:`GMT_DATASET <struct-dataset>`: Normally we allocate the output given the
corresponding input dimensions. You can override these by specifying your
alternative dimensions in the input dataset's variable ``dim[]``.
The ``data`` is a pointer to the resource you wish to duplicate. In case
of an error we return a NULL pointer and pass an error code via
``API->error``.

Convert between resource types
------------------------------

Having a resource in memory you may want to convert it to an alternative
representation.  For instance, you may have a :ref:`GMT_DATASET <struct-dataset>`
but need to strip the information from the
data into a VECTOR format, dropping all the segment header information, so
that your custom algorithm or other non-GMT functions can be used on the data.
In this case you will use

.. _GMT_Convert_Data:

  ::

    void *GMT_Convert_Data (void *API, void *In, unsigned int family_in,
		void *Out, unsigned int family_out, unsigned int flag[]);

which returns a pointer to the converted resource. Specify the needed
:ref:`family <tbl-family>` for both the input and output resources and set the
(up to) two flags passed via the ``flag`` array.  The first ``flag[0]``
determines how table headers and segment headers should be handled.
By default (``flag[0]`` = 0) they are preserved (to the extent possible).
E.g., converting a :ref:`GMT_DATASET <struct-dataset>` to MATRIX always means table headers are
skipped whereas segment headers are converted to NaN-records. Other
values for this flag is 1 (Table headers are not copied, segment headers are preserved),
2 (Headers are preserved, segment headers are reset to blank), or
3 (All headers headers are eliminated).  Note that this flag only
affects duplication of headers.  If the new object is written to file at
a later stage then it is up to the GMT default setting if headers are written
to file or not.
The second ``flag[1]`` controls restructuring of tables and segments within
a set.  For ``flag[1]`` = 0 we retain the original layout.  Other selections
are ``GMT_WRITE_TABLE_SEGMENT`` (combine all segments into a *single* segment in a *single* table),
``GMT_WRITE_TABLE`` (collect all segments into a *single* table), and ``GMT_WRITE_SEGMENT``
(combine segments into *one* segment per table).
Many family combinations are simply not allowed, such as grid to color palette, dataset to image,
etc.

Import Data Sets
----------------

If your program needs to import any of the six recognized data types
(data table, grid, image, cube, CPT, or PostScript) you will use
the GMT_Read_Data_ or GMT_Read_VirtualFile_ functions. The former
is typically used when reading from files, streams (e.g., ``stdin``), or
an open file handle, while the latter is only used to read from memory.
Because of the similarities of these six
import functions we use an generic form that covers all of them.

All input functions takes a parameter called ``mode``. The ``mode``
parameter generally has different meanings for the different data types
and will be discussed below. However, one bit setting is common to all
types: By default, you are only allowed to read a data source once; the
source is then flagged as having been read and subsequent attempts to
read from the same source will result in a warning and no reading takes
place. In the unlikely event you need to re-read a source you can
override this default behavior by adding ``GMT_IO_RESET`` to your ``mode``
parameter. Note that this override does not apply to sources that are
streams or file handles, as it may not be possible to re-read their
contents.


Import from a file, stream, or handle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To read an entire resource from a file, stream, or file handle, use

.. _GMT_Read_Data:

  ::

    void *GMT_Read_Data (void *API, unsigned int family, unsigned int method,
    	unsigned int geometry, unsigned int mode, double wesn[], const char *input, void *ptr);

* :ref:`API <GMT_Create_Session>`
* :ref:`family <tbl-family>`
* :ref:`method <tbl-methods>`
* :ref:`geometry <tbl-geometry>`
* mode -- *see below*
* :ref:`wesn <tbl-wesn>`
* input -- a pointer to char holding the file name to read, or NULL if ``stdin``
* ptr -- NULL or the pointer returned by this function after a first call (when reading grids in two steps)
* Return: Pointer to data container, or NULL if there were errors (passed back via API->error)


where ``ptr`` is NULL except when reading grids in two steps (i.e.,
first get a grid structure with a header, then read the data). Most of
these arguments have been discussed earlier. This function can be called
in three different situations:

#. If you have a single source (filename, stream pointer, etc.) you can
   call GMT_Read_Data_ directly; there is no need to first register
   the source with GMT_Register_IO_ or gather the sources with
   GMT_Init_IO_. Furthermore, for :ref:`GMT_DATASET <struct-dataset>` you can also
   specify a filename that contains UNIX wildcards (e.g., "all_*_[ab]?.txt")
   and these will all be read to produce a single multi-table :ref:`GMT_DATASET <struct-dataset>`
   (for other datatypes, see GMT_Read_Group_ instead).

#. If you want to specify ``stdin`` as source then pass ``input`` as NULL.

#. If you already registered all desired sources with GMT_Init_IO_
   then you indicate this choice by passing the invalid ``geometry`` = 0.

Space will be allocated to hold the results, as needed, and a pointer to
the object is returned. If there are errors we simply return NULL and
report the error. Note that you can read in a GMT_IS_MATRIX either from a text
table (passing ``geometry`` as GMT_IS_POINT) or from a grid (passing ``geometry``
as GMT_IS_SURFACE).  The ``mode`` parameter has different meanings for
different data types.

**Color palette table**.
    ``mode`` contains bit-flags that control how the CPT's back-,
    fore-, and NaN-colors should be initialized. Select 0 to use the
    CPT resource's back-, fore-, and NaN-colors, 2 to replace these with the current
    GMT default values, or 4 to replace them with the color table's
    entries for highest and lowest value.

**Data table**.
    ``mode`` is currently not used.

**Text table**.
    ``mode`` is currently not used.

**GMT grid** or **image**.
    Here, ``mode`` determines how we read the grid: To read the entire
    grid and its header, pass ``GMT_CONTAINER_AND_DATA``. However, if you may need to
    extract a sub-region you must first read the header by passing
    ``GMT_CONTAINER_ONLY`` with ``wesn`` = NULL, then examine the header structure range
    attributes, specify a subset via the array ``wesn``, and
    finally call GMT_Read_Data_ a second time, now with ``mode`` =
    ``GMT_DATA_ONLY``, passing your ``wesn`` array and the grid
    structure returned from the first call as ``ptr``. In the event your
    data array should be allocated to hold both the real and imaginary
    parts of a complex data set you must add either
    ``GMT_GRID_IS_COMPLEX_REAL`` or ``GMT_GRID_IS_COMPLEX_IMAG`` to
    ``mode`` so as to allow for the extra memory needed and to stride
    the complex value-pairs correctly. If your grid is huge and you must read
    it row-by-row, set ``mode`` to ``GMT_CONTAINER_ONLY`` \|
    ``GMT_GRID_ROW_BY_ROW``. You can then access the grid row-by-row
    using GMT_Get_Row_. By default, the rows will be automatically
    processed in sequential order. To completely specify which row to be read, pass
    ``GMT_GRID_ROW_BY_ROW_MANUAL`` instead.
    Finally, as an option you may add ``GMT_GRID_XY`` to the mode and we also
    allocate the *x* and *y* coordinate vectors for the grid or image.

*PostScript*.
    ``mode`` is currently not used.

If you need to read the same resource more than once you should add the
bit flag ``GMT_IO_RESET`` to the given ``mode``.

Import a group of data sets
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To read a group of resources, you may instead use

.. _GMT_Read_Group:

  ::

    void *GMT_Read_Group (void *API, unsigned int family, unsigned int method,
    	unsigned int geometry, unsigned int mode, double wesn[],
    	void *input, unsigned int *n_items, void *ptr);

* :ref:`API <GMT_Create_Session>`
* :ref:`family <tbl-family>`
* :ref:`method <tbl-methods>`
* :ref:`geometry <tbl-geometry>`
* mode -- *see below*
* :ref:`wesn <tbl-wesn>`
* input -- Contents depends on the value of *n_items*.  If it is zero then we expect
  a pointer to char holding UNIX wildcard file name(s) to read, otherwise we expect
  a pointer to an array of character strings (*n_items* in total) with names of all
  the files to read.  If *n_items* is NULL then we assume 0 but cannot return the number
  found.
* ptr -- NULL or the pointer returned by this function after a first call (applies when reading grids or images in two steps)
* Return: Pointer to array of data container, or NULL if there were errors (passed back via API->error)


where ``ptr`` is NULL except when reading grids in two steps (i.e.,
first get a grid structures with a header, then read the data arrays). Most of
these arguments have been discussed earlier. It is useful when you need to read
a series of files (e.g., from a list with filenames) or want to specify the items
to read using a UNIX wildcard specification.  **Note**: If used with :ref:`GMT_DATASET <struct-dataset>`
then you will receive an array of structures as well.  Typically, many data files
are read into separate tables that all form part of a single SET (this is what GMT_Read_Data_ does),
but if GMT_Read_Group_ is used on the same arguments then an array of one-table sets will
be returned instead.  The purpose of your application will dictate which form is more convenient.

Using user arrays in GMT
~~~~~~~~~~~~~~~~~~~~~~~~

If your program uses a matrix or a set of column vectors to hold data
and you wish to use such data in a GMT module, you must first create a
GMT_MATRIX (for matrices) or GMT_VECTOR (for vectors) to hold your arrays.
In this situation you must pass ``dim`` with the final dimensions of
your rows and columns when you call GMT_Create_Data_ to make the empty
containers.  You can then use GMT_Put_Matrix_ and GMT_Put_Vector_ to hook
up your own allocated arrays.  It is then these containers that you
will pass to GMT via *virtual files*. For receiving output from GMT it is
normal to simply use Open_VirtualFile and have GMT allocate the space needed.
However, if you want the result to be written to your own arrays or matrix
then you must call GMT_Create_Data yourself with mode = GMT_IS_OUTPUT and
specify the dimensions of your array, then (as for input) assign your memory
to the container using GMT_Put_Matrix_ or GMT_Put_Vector_.  Finally, if
you also need to pass record of strings then see GMT_Put_Strings_ and
GMT_Get_Strings_.

Open a virtual file (memory location)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you have read in or otherwise obtained a data object in memory and you
now wish for it to serve as input to a GMT module, you will have to associate
that object with a "Virtual File".  This step assigns a special filename to the
memory location and you can then pass this filename to any module that
needs to read that data.  It is similar for writing, except you may pass
NULL as the object to have GMT automatically allocate the output resource.
If you want GMT to write to your preallocated memory then you must instead create a
suitable container first (and pass the dimensions of the arrays) and then
attach your array(s) using GMT_Put_Matrix_ or GMT_Put_Vector_.
The full syntax is

.. _GMT_Open_VirtualFile:

  ::

    int GMT_Open_VirtualFile (void *API, unsigned int family, unsigned int geometry,
		unsigned int direction, void *data, char *filename);

Here, ``data`` is the pointer to your memory object.  The function returns the
desired filename via ``filename``.  This string must be at least ``GMT_VF_LEN`` bytes (16).
The other arguments have been discussed earlier.  Specifically for direction, use
GMT_IN for reading and GMT_OUT for writing.  Simply pass this filename in
the calling sequence to the module you want to use to indicate which file should
be used for reading or writing.  Note that if you plan to pass a matrix or vectors
instead of grids or dataset you must add the modifiers GMT_IS_MATRIX or GMT_IS_VECTOR
to ``family`` so that the module knows what to do.  Finally, in the case of passing
``data`` as NULL you may also control what type of matrix or vector will be created in
GMT for the output by adding in the modifiers GMT_VIA_type, as listed in :ref:`types <tbl-viatypes>`.
**Note**: GMT tries to minimize data duplication if possible, so if your input arrays are
compatible with the data type used by the modules then we could use your array directly.
This *may* have the side-effect that your input array is modified by the module, especially
if the module writes the results to a netCDF grid file.
If that is a price you are willing to pay then you can add GMT_IS_REFERENCE to the ``direction``
argument and we will pass the array internally to avoid duplicating memory. For output it is
best to pass GMT_IS_REFERENCE as well.

Import from a virtual file
~~~~~~~~~~~~~~~~~~~~~~~~~~

Once the module completes it will have written its output to the virtual file
you initialized with GMT_Open_VirtualFile_.  To use the actual
data you will need to "read" it into your program.  Of course, the data are already
in memory but to access it you need to use GMT_Read_VirtualFile_, which expects
the output filename you obtained from GMT_Open_VirtualFile_.  The syntax is

.. _GMT_Read_VirtualFile:

  ::

    void *GMT_Read_VirtualFile (void *API, char *filename);

The function requires the output filename via ``filename`` and then returns
the data object, similar to what GMT_Read_Data_ does.

Inquire a virtual file for family
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you do not know what family is being represented by a virtual file
then you should first obtain the family via GMT_Inquire_VirtualFile_.  The syntax is

.. _GMT_Inquire_VirtualFile:

  ::

    int GMT_Inquire_VirtualFile (void *API, const char *filename);

The function requires the virtual file's ``filename`` and then returns the
family of the data object.

Reset a virtual file for reuse
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Should you need to read a virtual file again then you must first reset
it to its original state with GMT_Init_VirtualFile_.  The syntax is

.. _GMT_Init_VirtualFile:

  ::

    int GMT_Init_VirtualFile (void *API, unsigned int mode, const char *filename);

The function requires the virtual file's ``filename`` and then resets the
internal counters (e.g., record numbers and other book-keeping parameters).
The ``mode`` is presently not used.

Close a virtual file
~~~~~~~~~~~~~~~~~~~~

Once you have finished using a virtual file you need to close it.
This will reset its internal settings back to what it was before you
used it as a virtual file.  The syntax is


.. _GMT_Close_VirtualFile:

  ::

    int GMT_Close_VirtualFile (void *API, char *filename);

where ``filename`` is the name of the virtual file.


Record-by-record input
----------------------

In the case of data tables you have the option of selecting
record-by-record reading or writing.  As a general rule, your program
development simplifies if you can read entire resources into memory with
GMT_Read_Data_ or GMT_Read_VirtualFile_.  However, if this leads to
unacceptable memory usage or if the program logic is particularly simple,
you may obtain one data record at the time via GMT_Get_Record_ and write
one at the time with GMT_Put_Record_.  For row-by-row i/o for grids there
is the corresponding function GMT_Get_Row_. There are additional overhead involved
in setting up record-by-record processing, which is the topic of this section.

Enable Data Import
~~~~~~~~~~~~~~~~~~

Once all input resources have been registered, we signal the API that we
are done with the registration phase and are ready to start the actual
data import. This step is only required when reading one record at the
time. We initialize record-by-record reading by calling
GMT_Begin_IO_. This function enables data
record-by-record reading and prepares the registered sources for the
upcoming import. The prototype is

.. _GMT_Begin_IO:

  ::

    int GMT_Begin_IO (void *API, unsigned int family, unsigned int direction,
    	unsigned int header);

where :ref:`family <tbl-family>` specifies the resource type to be read or written
(only ``GMT_IS_DATASET`` is
available for record-by-record handling). The ``direction`` is either
``GMT_IN`` or ``GMT_OUT``, so for import we obviously use ``GMT_IN``. The
function determines the first input source and sets up procedures for
skipping to the next input source in a virtual data set. The
GMT_Get_Record_ function will not be able to read any data before
GMT_Begin_IO_ has been called. As you might guess, there is a
companion GMT_End_IO_ function that completes, then disables
record-by-record data access. You can use these several times to switch
modes between registering data resources, doing the importing/exporting,
and disabling further data access, perhaps to do more registration. We
will discuss GMT_End_IO_ once we are done with the data import. The final
``header`` argument determines if the common header-block should be
written during initialization; choose between ``GMT_HEADER_ON`` and
``GMT_HEADER_OFF``. The function returns 1 if there is an
error; otherwise it returns 0.

Set data geometry
~~~~~~~~~~~~~~~~~

Typically only done for output data written record by record, we designate
the data set's geometry by calling

.. _GMT_Set_Geometry:

  ::

    int _GMT_Set_Geometry (void *API,  unsigned int direction, unsigned int geometry);

where ``direction`` is either ``GMT_IN`` or ``GMT_OUT`` and :ref:`geometry <tbl-geometry>`
sets the geometry that will be produced (or read).


Importing a data record
~~~~~~~~~~~~~~~~~~~~~~~

If your program will read data table records one-by-one you must first
enable this input mechanism with GMT_Begin_IO_ and then read the
records within a loop, repeatedly using

.. _GMT_Get_Record:

  ::

    void *GMT_Get_Record (void *API, unsigned int mode, int *nfields);

where the returned value is a pointer to a GMT_RECORD structure, whose
member pointers data and text point to ephemeral memory
internal to GMT and should be considered read-only. When we reach
end-of-file, encounter conversion problems, read header comments, or
identify segment headers we instead return a NULL pointer. The ``nfields``
integer pointer will return the number of fields returned; pass NULL if your
program should ignore this information.

Normally (i.e., ``mode`` = ``GMT_READ_DATA``), we return a pointer to
a double array. To read text records, supply instead ``mode`` =
``GMT_READ_TEXT`` and we will return a pointer to the text
record. However, if you have input records that mixes organized
floating-point columns with text items you could pass ``mode`` =
``GMT_READ_MIXED``. Then, GMT will attempt to extract the
floating-point values from as many columns as needed; you can still access the original record string, as
discussed below. Finally, if your application needs to be notified when
GMT closes one file and opens the next, add ``GMT_FILE_BREAK`` to
``mode`` and check for the status code ``GMT_IO_NEXT_FILE`` (by default,
we treat the concatenation of many input files as a single virtual
file). Using GMT_Get_Record_ requires you to first initialize the
source(s) with GMT_Init_IO_. For certain records, GMT_Get_Record_
will return NULL and sets status codes that your program will need to
examine to take appropriate response. Table :ref:`IO-status <tbl-iostatus>` lists the
various status codes you can check for, using the ``GMT_Get_Status`` function (see
next section).

Examining record status
~~~~~~~~~~~~~~~~~~~~~~~

Programs that read record-by-record must be aware of what the current
record represents. Given the presence of headers, data gaps, NaN-record,
etc., the developer may want to check the status after reading the current
record. The internal i/o status mode can be interrogated with the function

.. _GMT_Get_Status:

  ::

    int GMT_Get_Status (void *API, unsigned int mode);

which returns 0 (false) or 1 (true) if the current status is reflected
by the specified ``mode``. There are 11 different modes available to
programmers; for a list see Table :ref:`IO-status <tbl-iostatus>` For an example of how
these may be used, see the test program ``testgmtio.c``. Developers who plan to import
data on a record-by-record basis may also consult the source code of,
say, :doc:`blockmean` or :doc:`text`, to see examples of working code.

.. _tbl-iostatus:

    +-----------------------+--------------------------------------------------------+
    | mode                  | description and return value                           |
    +=======================+========================================================+
    | GMT_IO_DATA_RECORD    | 1 if we read a data record                             |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_TABLE_HEADER   | 1 if we read a table header                            |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_SEGMENT_HEADER | 1 if we read a segment header                          |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_ANY_HEADER     | 1 if we read either header record                      |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_MISMATCH       | 1 if we read incorrect number of columns               |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_EOF            | 1 if we reached the end of the file (EOF)              |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_NAN            | 1 if we only read NaNs                                 |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_GAP            | 1 if this record implies a data gap                    |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_NEW_SEGMENT    | 1 if we enter a new segment                            |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_LINE_BREAK     | 1 if we encountered a segment header, EOF, NaNs or gap |
    +-----------------------+--------------------------------------------------------+
    | GMT_IO_NEXT_FILE      | 1 if we finished one file but not the last             |
    +-----------------------+--------------------------------------------------------+

    The various modes used to test the status of the record-by-record machinery.

Importing a grid row
~~~~~~~~~~~~~~~~~~~~

If your program must read a grid file row-by-row you must first enable
row-by-row reading with GMT_Read_Data_ and then use the
GMT_Get_Row_ function in a loop; the prototype is

.. _GMT_Get_Row:

  ::

    int GMT_Get_Row (void *API, int row_no, struct GMT_GRID *G, float *row);

where ``row`` is a pointer to a pre-allocated single-precision array to receive the
current row, ``G`` is the grid in question, and ``row_no`` is the number
of the current row to be read. Note this value is only considered if the
row-by-row mode was initialized with ``GMT_GRID_ROW_BY_ROW_MANUAL``.
The user must allocate enough space to hold the entire row in memory.

Disable Data Import
~~~~~~~~~~~~~~~~~~~

Once the record-by-record input processing has completed we disable
further input to prevent accidental reading from occurring (due to poor
program structure, bugs, etc.). We do so by calling GMT_End_IO_. This
function disables further record-by-record data import; its prototype is

.. _GMT_End_IO:

  ::

    int GMT_End_IO (void *API, unsigned int direction, unsigned int mode);

and we specify ``direction`` = ``GMT_IN``. At the moment, ``mode`` is not
used. This call will also reallocate any arrays obtained into their
proper lengths. The function returns 1 if there is an error
(whose code is passed back with ``API->error``), otherwise it returns 0 (``GMT_NOERROR``).

.. _sec-manipulate:

Manipulate data
---------------

Once you have created and allocated empty resources, or read in
resources from the outside, you may wish to manipulate their contents.
This section discusses how to set up loops and access the important
variables for each of the supported families. For grids and images it may in addition
be required to determine what the coordinates are at each node point. This information
can be obtained via arrays of coordinates for each dimension, obtained by

.. _GMT_Get_Coord:

  ::

    double *GMT_Get_Coord (void *API, unsigned int family, unsigned int dim,
    	void *data);

where :ref:`family <tbl-family>` must be ``GMT_IS_GRID`` or ``GMT_IS_DATASET``, ``dim`` is either
``GMT_IS_X`` or ``GMT_IS_Y``, and ``data`` is the grid or image pointer.  This
function will be used below in our example on grid manipulation.

Another aspect of dealing with grids and images is to convert a row and column
2-D reference to our 1-D array index.  Because of grid and image boundary padding
the relationship is not straightforward, hence we supply

.. _GMT_Get_Index:

  ::

    uint64_t GMT_Get_Index (struct GMT_GRID_HEADER *header, int row, int col);

where the ``header`` is the header of either a grid or image, and ``row`` and
``col`` is the 2-D position in the grid or image.  We return the 1-D array
position; again this function is used below in our example.  Likewise, for images
with many layers we also define

.. _GMT_Get_Pixel:

  ::

    uint64_t GMT_Get_Pixel (struct GMT_GRID_HEADER *header, int row,
    	int col, int layer);

where the ``header`` is the header of an image, and ``row``, ``col`` and
``layer`` (= 1 for grids) is the position in the grid or image.

For data cubes we need to also supply the ``level`` in the cube. Because
each layer is basically a padded grid, we supply

.. _GMT_Get_Index3:

  ::

    uint64_t GMT_Get_Index3 (struct GMT_GRID_HEADER *header, int row, int col, int level);

where we return the 1-D array position.

Manipulate grids
~~~~~~~~~~~~~~~~

Most applications wishing to manipulate grids will want to loop over all
the nodes, typically in a manner organized by rows and columns. In doing
so, the coordinates at each node may also be required for a calculation.
Below is a snippet of code that shows how to do visit all nodes in a
grid and assign each node the product x \* y:

  ::

    int row, col, node;
    double *x_coord = NULL, *y_coord = NULL;
    /*... create a grid G or read one ... */
    x_coord = GMT_Get_Coord (API, GMT_IS_GRID, GMT_X, G);
    y_coord = GMT_Get_Coord (API, GMT_IS_GRID, GMT_Y, G);
    for (row = 0; row < G->header->n_rows) {
        for (col = 0; col < G->header->n_columns; col++) {
            node = GMT_Get_Index (G->header, row, col);
            G->data[node] = x_coord[col] * y_coord[row];
        }
    }

Note the use of GMT_Get_Index_ to get the grid node number associated
with the ``row`` and ``col`` we are visiting. Because GMT grids have
padding (for boundary conditions) the relationship between rows,
columns, and node indices is more complicated and hence we hide that
complexity in GMT_Get_Index_. Note that for trivial procedures such
setting all grid nodes to a constant (e.g., -9999.0) where the row and
column does not enter you can instead do a single loop:

  ::

    int node;
    /*... create a grid G or read one ... */
    for (node = 0; node < G->header->size) G->data[node] = -9999.0;

Note we must use ``G->header->size`` (size of allocated array) and not
``G->header->nm`` (number of nodes in grid) since the latter is smaller
due to the padding and a single loop like the above treats the pad as
part of the "inside" grid. Replacing ``size`` by ``nm`` would be a bug.

Manipulate data tables
~~~~~~~~~~~~~~~~~~~~~~

Another common application is to process the records in a data table.
Because GMT considers the :ref:`GMT_DATASET <struct-dataset>` resources to contain one or more
tables, each of which may contain one or more segments, all of which may
contain one or more columns, you will need to have multiple nested loops to
visit all entries. The following code snippet will visit all data
records and add 1 to all columns beyond the first two (x and y), and if
the data has a trailing string it will print it to stdout:

  ::

    uint64_t tbl, seg, row, col;
    struct GMT_DATATABLE *T = NULL;
    struct GMT_DATASEGMENT *S = NULL;

    /* ... create a dataset D or read one ... */
    for (tbl = 0; tbl < D->n_tables; tbl++) {       /* For each table */
      T = D->table[tbl];       /* Convenient shorthand for current table */
      for (seg = 0; seg < T->n_segments; seg++) {   /* For all segments */
        S = T->segment[seg];   /* Convenient shorthand for current segment */
        for (row = 0; row < S->n_rows; row++) {	/* For all rows in segment */
          for (col = 2; col < T->n_columns; col++) {	/* For all cols > 1 */
            S->data[col][row] += 1.0;	/* Just add one */
          }
		  if (S->text) printf ("Row %d has string: %s\n", (int)row, S->text[row]);
        }
      }
    }

Message and Verbose Reporting
-----------------------------

The API provides two functions for your program to present information
to the user during the run of the program. One is used for messages that
are always written (optionally with a time stamp) while the other is used
for reports whose verbosity level must exceed the verbosity settings specified via **-V**.

Verbose reporting
~~~~~~~~~~~~~~~~~

.. _GMT_Report:

  ::

    int GMT_Report (void *API, unsigned int level, const char *message, ...);

This function takes a verbosity level and a multi-part message (e.g., a
format statement and zero or more variables as required by the format string). The verbosity ``level`` is
an integer in the 0–5 range; these levels are listed in Table :ref:`timemodes <tbl-verbosity>`
You assign an appropriate verbosity level to your message, and depending
on the chosen run-time verbosity level set via **-V** your message may
or may not be reported. Only messages whose stated verbosity level is
lower or equal to the **-V**\ *level* will be printed.  These messages are typically
progress reports, etc., and are sent to standard error.


.. _tbl-verbosity:

    +----------------------+--------------------------------------+
    | constant             | description                          |
    +======================+======================================+
    | GMT_MSG_QUIET        | Quiet; no messages whatsoever        |
    +----------------------+--------------------------------------+
    | GMT_MSG_ERROR        | Error messages only                  |
    +----------------------+--------------------------------------+
    | GMT_MSG_WARNING      | Warnings                             |
    +----------------------+--------------------------------------+
    | GMT_MSG_TICTOC       | Time usage for slow algorithms       |
    +----------------------+--------------------------------------+
    | GMT_MSG_INFORMATION  | Informational messages               |
    +----------------------+--------------------------------------+
    | GMT_MSG_COMPAT       | Compatibility warnings               |
    +----------------------+--------------------------------------+
    | GMT_MSG_DEBUG        | Debug messages for developers mostly |
    +----------------------+--------------------------------------+

    The different levels of verbosity that can be selected.

Error string
~~~~~~~~~~~~

.. _GMT_Error_Message:

  ::

    char * GMT_Error_Message (void *API);

This function simply returns a character pointer to the internal error message
buffer holding the last error message generated.

User messages
~~~~~~~~~~~~~

For custom messages to the user that should always be printed, we use

.. _GMT_Message:

  ::

    int GMT_Message (void *API, unsigned int mode, const char *format, ...);

This function always prints its message to the standard output. Use the
``mode`` value to control if a time stamp should preface the message,
and if selected how the time information should be formatted. See
Table :ref:`timemodes <tbl-timemodes>` for the various modes.

.. _tbl-timemodes:

    +------------------+---------------------------------------+
    | constant         | description                           |
    +==================+=======================================+
    | GMT_TIME_NONE    | Display no time information           |
    +------------------+---------------------------------------+
    | GMT_TIME_CLOCK   | Display current local time            |
    +------------------+---------------------------------------+
    | GMT_TIME_ELAPSED | Display elapsed time since last reset |
    +------------------+---------------------------------------+
    | GMT_TIME_RESET   | Reset the elapsed time to 0           |
    +------------------+---------------------------------------+

    The different types of message modes.

Special GMT modules
-------------------

There are some differences between calling
modules on the command line and using them via the API.  These are discussed here.

API-only modules
~~~~~~~~~~~~~~~~

There are two general-purpose modules that are not part of the command-line version of
GMT.  These are the read and write modules.  Both take an option to specify what GMT
resource is being read of written: **-Tc**\|\ **d**\|\ **g**\|\ **i**\|\ **p**,
which selects CPT, dataset, grid, image, or PostScript, respectively.  In addition
both modules accept the *infile* and *outfile* argument for source and destination.  These
may be actual files of memory locations, of course.

PostScript Access
~~~~~~~~~~~~~~~~~

The GMT module :doc:`psconvert` is normally given one or more PostScript files that may be
converted to other formats.  When accessed by the API it may also be given the special
file name "=", which means we are to use the internal PostScript string produced by
the latest GMT plotting instead of any actual file name.  The module can access this
string which must be a complete plot (i.e., it must have header, middle, and trailer
and thus be a valid PostScript file).  This allows the API to convert plots to a
suitable image format without any duplication and manipulation of the PostScript
itself.

Adjusting headers and comments
------------------------------

All header records in incoming datasets are stored in memory. You may
wish to replace these records with new information, or append new
information to the existing headers. This is achieved with

.. _GMT_Set_Comment:

  ::

    int GMT_Set_Comment (void *API, unsigned int family, unsigned int mode,
    	void *arg, void *data)

Again, :ref:`family <tbl-family>` selects which kind of resource is passed via ``data``.
The ``mode`` determines what kind of comment is being considered, how it
should be included, and in what form the comment passed via ``arg`` is provided.
Table :ref:`comments <tbl-comments>` lists the available options, which may be combined
by adding (bitwise "or"). The GMT_Set_Comment_ function does not actually
output anything but sets the relevant comment and header records in the
relevant structure. When a file is written out the information will be
output as well (**Note**: Users can always decide if they wish to turn
header output on or off via the common GMT option ``-h``. For
record-by-record writing you must enable the header block output when
you call GMT_Begin_IO_.

.. _tbl-comments:

    +-------------------------+---------------------------------------------------+
    | constant                | description                                       |
    +=========================+===================================================+
    | GMT_COMMENT_IS_TEXT     | Comment is a text string                          |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_OPTION   | Comment is a linked list of GMT_OPTION structures |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_COMMAND  | Comment is the command                            |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_REMARK   | Comment is the remark                             |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_TITLE    | Comment is the title                              |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_NAME_X   | Comment is the x variable name (grids only)       |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_NAME_Y   | Comment is the y variable name (grids only)       |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_NAME_Z   | Comment is the z variable name (grids only)       |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_COLNAMES | Comment is the column names header                |
    +-------------------------+---------------------------------------------------+
    | GMT_COMMENT_IS_RESET    | Comment replaces existing information             |
    +-------------------------+---------------------------------------------------+

    The modes for setting various comment types.

The named modes (*command*, *remark*, *title*, *name_x,y,z* and
*colnames* are used to distinguish regular text comments from specific
fields in the header structures of the data resources, such as
:ref:`GMT_GRID <struct-grid>`. For the various table resources (e.g., :ref:`GMT_DATASET <struct-dataset>`)
these modifiers result in a specially formatted comments beginning with
"Command: " or "Remark: ", reflecting how this type of information is
encoded in the headers.

Export Data Sets
----------------

If your program needs to write any of the six recognized data types
(CPTs, data tables, grids, images, cubes or PostScript) you can use the
GMT_Write_Data_ function.

Both of these output functions takes a parameter called ``mode``. The
``mode`` parameter generally takes on different meanings for the
different data types and will be discussed below. However, one bit
setting is common to all types: By default, you are only allowed to
write a data resource once; the resource is then flagged to have been
written and subsequent attempts to write to the same resource will
quietly be ignored. In the unlikely event you need to re-write a
resource you can override this default behavior by adding ``GMT_IO_RESET``
to your ``mode`` parameter.

Exporting a data set
~~~~~~~~~~~~~~~~~~~~

To have your program accept results from GMT modules and write them
separately requires you to use the GMT_Write_Data_ function. It is very similar to the
GMT_Read_Data_ function encountered earlier.

Exporting a data set to a file, stream, or handle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The prototype for writing to a file (via name, stream, or file handle) is

.. _GMT_Write_Data:

  ::

    int GMT_Write_Data (void *API, unsigned int family, unsigned int method,
    	unsigned int geometry, unsigned int mode, double wesn[], void *output, void *data);

* :ref:`API <GMT_Create_Session>`
* :ref:`family <tbl-family>`
* :ref:`method <tbl-methods>`
* :ref:`geometry <tbl-geometry>`
* mode -- specific to each data type (\ *see below*)
* :ref:`wesn <tbl-wesn>`
* output --
* data -- A pointer to any of the six families.
* Return: 0 on success, otherwise return -1 and set API->error to reflect to cause.

where ``data`` is a pointer to any of the four structures discussed previously.

**Color palette table**
    ``mode`` controls if the CPT's back-, fore-, and NaN-colors
    should be written (1) or not (0).

**Data table**
    If ``method`` is ``GMT_IS_FILE``, then the value of ``mode`` affects
    how the data set is written:

    **GMT_WRITE_SET**
        The entire data set will be written to the single file [0].

    **GMT_WRITE_TABLE**
        Each table in the data set is written to individual files [1].
        You can either specify an output file name that *must* contain
        one C-style format specifier for an int variable (e.g.,
        "New_Table_%06d.txt"), which will be replaced with the table
        number (a running number from 0) *or* you must assign to each
        table *i* a unique output file name via the
        ``D->table[i]->file[GMT_OUT]`` variables prior to calling the
        function.

    **GMT_WRITE_SEGMENT**
        Each segment in the data set is written to an individual file
        [2]. Same setup as for ``GMT_WRITE_TABLE`` except we use
        sequential segment numbers to build the file names.

    **GMT_WRITE_TABLE_SEGMENT**
        Each segment in the data set is written to an individual file
        [3]. You can either specify an output file name that *must*
        contain two C-style format specifiers for two int variables
        (e.g., "New_Table_%06d_Segment_%03d.txt"), which will be
        replaced with the table and segment numbers, *or* you must
        assign to each segment *j* in each table *i* a unique output
        file name via the ``D->table[i]->segment[j]->file[GMT_OUT]``
        variables prior to calling the function.

    **GMT_WRITE_OGR**
        Writes the dataset in OGR/GMT format in conjunction with the
        ``-a`` setting [4].

**Text table**
    The ``mode`` is used the same way as for data tables.

**GMT grid**
    Here, ``mode`` may be ``GMT_CONTAINER_ONLY`` to only update a
    file's header structure, but normally it is simply ``GMT_CONTAINER_AND_DATA``
    so the entire grid and its header will be exported (a subset is
    not allowed during export). However, in the event your data array
    holds both the real and imaginary parts of a complex data set you
    must add either ``GMT_GRID_IS_COMPLEX_REAL`` or
    ``GMT_GRID_IS_COMPLEX_IMAG`` to ``mode`` so as to export the
    corresponding grid values correctly. Finally, for native binary
    grids you may skip writing the grid header by adding
    ``GMT_GRID_NO_HEADER``; this setting is ignored for all other grid
    formats. If your output grid is huge and you are building it
    row-by-row, set ``mode`` to ``GMT_CONTAINER_ONLY`` \|
    ``GMT_GRID_ROW_BY_ROW``. You can then write the grid row-by-row
    using GMT_Put_Row_. By default the rows will be automatically
    processed in order. To completely specify which row to be written,
    use ``GMT_GRID_ROW_BY_ROW_MANUAL`` instead; this requires a file format
    that supports direct writes, such as netCDF.  Finally, if you are
    preparing a geographic grid outside of GMT you need to add the mode
    ``GMT_GRID_IS_GEO`` to ensure that the proper metadata will be written
    to the netCDF header, thus letting the grid be recognized as such.

**Note**: If ``method`` is GMT_IS_FILE, :ref:`family <tbl-family>` is ``GMT_IS_GRID``,
and the filename implies a change from NaN to another value then the grid is
modified accordingly. If you continue to use that grid after writing please be
aware that the changes you specified were applied to the grid.

Record-by-record output
-----------------------

In the case of data tables, you may also
consider the GMT_Put_Record_ function for record-by-record writing. As a general rule, your
program organization may simplify if you can write the entire
resource with GMT_Write_Data_. However, if the program logic is simple
or already involves using GMT_Get_Record_, it may be better to export
one data record at the time via GMT_Put_Record_.  For grids there is the
corresponding GMT_Put_Row_ function.

Enable Data Export
~~~~~~~~~~~~~~~~~~

Similar to the data import procedures, once all output destinations have
been registered, we signal the API that we are done with the
registration phase and are ready to start the actual data export. As for
input, this step is only needed when dealing with record-by-record
writing. Again, we enable record-by-record writing by calling
GMT_Begin_IO_, this time with ``direction`` = ``GMT_OUT``. This function
enables data export and prepares the registered destinations for the
upcoming writing.


Specifying the number of output columns
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For record-based ASCII input/output you will need to specify the number of
columns, unless for output it equals the number of input columns.  This is done with
the GMT_Set_Columns_ function:

.. _GMT_Set_Columns:

  ::

    void *GMT_Set_Columns (void *API, unsigned int direction, unsigned int n_columns, unsigned int mode);

The ``n_columns`` is a number related to the number of columns you plan to read/write, while
``mode`` controls what that number means.  For input, ``mode`` = ``GMT_COL_FIX`` sets the actual
number of numerical columns to read.  Anything beyond is considered trailing text and is parsed unless
you use ``GMT_COL_FIX_NO_TEXT`` instead.  If your records have variable number of numerical columns
then you may use ``GMT_COL_VAR``. For output, you can also select from
other modes.  Here,  ``mode`` = ``GMT_COL_ADD`` means it should be added to the known number
of input columns to arrive at the number of final output columns, while ``mode`` = ``GMT_COL_SUB``
means this value should be subtracted from the number of input columns to find the number of
output columns.


Exporting a data record
~~~~~~~~~~~~~~~~~~~~~~~

If your program must write data table records one-by-one you must first
enable record-by-record writing with GMT_Begin_IO_ and then use the
``GMT_Put_Record`` function in a loop; the prototype is

.. _GMT_Put_Record:

  ::

    int GMT_Put_Record (void *API, unsigned int mode, void *rec);

where ``rec`` is a pointer to (a) a GMT_RECORD structure for
the current row. Alternatively (b), ``rec``
points to a text string. The ``mode`` parameter must be set to reflect
what is passed. Using GMT_Put_Record_ requires you to first
initialize the destination with GMT_Init_IO_. Note that for
``GMT_IS_DATASET``  the methods ``GMT_IS_DUPLICATE`` and
``GMT_IS_REFERENCE`` are not supported since you can simply populate the
:ref:`GMT_DATASET <struct-dataset>` structure directly. As mentioned, ``mode`` affects what is
actually written:

**GMT_WRITE_DATA**.
    Normal operation that builds the current output record from the numerical values in ``rec``.

**GMT_WRITE_TABLE_HEADER**.
    For ASCII output mode we write the text string ``rec``. If ``rec``
    is NULL then we write the last read header record. If binary
    output mode we quietly skip writing this record.

**GMT_WRITE_SEGMENT_HEADER**.
    For ASCII output mode we use the text string ``rec`` as the
    segment header. If ``rec`` is NULL then we use the current (last
    read) segment header record. If binary output mode instead we write
    a record composed of NaNs.

The function returns 1 if there was an error associated with the
writing (which is passed back with ``API->error``), otherwise it returns
0 (``GMT_NOERROR``).

Exporting a grid row
~~~~~~~~~~~~~~~~~~~~

If your program must write a grid file row-by-row you must first enable
row-by-row writing with GMT_Read_Data_ and then use the
GMT_Put_Row_ function in a loop; the prototype is

.. _GMT_Put_Row:

  ::

    int GMT_Put_Row (void *API, int row_no, struct GMT_GRID *G, float *row);

where ``row`` is a pointer to a single-precision array with the current
row, ``G`` is the grid in question, and ``row_no`` is the number of the
current row to be written. Note this value is only considered if the
row-by-row mode was initialized with ``GMT_GRID_ROW_BY_ROW_MANUAL``.

Disable Data Export
~~~~~~~~~~~~~~~~~~~

Once the record-by-record output has completed we disable further output
to prevent accidental writing from occurring (due to poor program
structure, bugs, etc.). We do so by calling GMT_End_IO_. This
function disables further record-by-record data export; here, we
obviously pass ``direction`` as ``GMT_OUT``.

Destroy allocated resources
---------------------------

If your session imported any data sets into memory then you may
explicitly free this memory once it is no longer needed and before
terminating the session. This is done with the GMT_Destroy_Data_
function, whose prototype is

.. _GMT_Destroy_Data:

  ::

    int GMT_Destroy_Data (void *API, void *data);

where ``data`` is the address of the pointer to a data container, i.e., not
the pointer to the container but the *address* of that pointer (e.g. &pointer).  Note that
when each module completes it will automatically free memory created by
the API; similarly, when the session is destroyed we also automatically
free up memory. Thus, ``GMT_Destroy_Data`` is therefore generally only
needed when you wish to directly free up memory to avoid running out of
it. The function returns 1 if there is an error when trying to
free the memory (the error code is passed back with ``API->error``),
otherwise it returns 0 (``GMT_NOERROR``).

Destroy groups of allocated resources
-------------------------------------

If you obtained an array of resources via GMT_Read_Group_ then
you will need to destroy these resources with GMT_Destroy_Group_ instead,
whose prototype is

.. _GMT_Destroy_Group:

  ::

    int GMT_Destroy_Group (void *API, void *data, unsigned int n);

where ``data`` is the address of the array with data containers, i.e., not
the array to the containers but the *address* of that array (e.g. &array),
and ``n`` is the number of containers.

Terminate a GMT session
-----------------------

Before your program exits it should properly terminate the
GMT session, which involves a call to

.. _GMT_Destroy_Session:

  ::

    int GMT_Destroy_Session (void *API);

which simply takes the pointer to the GMT API control structure as its
only arguments. It terminates the GMT machinery and deallocates all
memory used by the GMT API book-keeping. It also unregisters any
remaining resources previously registered with the session. The
GMT API will only close files that it was responsible for opening in
the first place. Finally, the API structure itself is freed so your main
program does not need to do so. The function returns 1 if there
is an error when trying to free the memory (the error code is passed
back with ``API->error``), otherwise it returns 0 (``GMT_NOERROR``).

.. _sec-parsopt:

Presenting and accessing GMT options
------------------------------------

As you develop a program you may wish to rely on some of
the GMT common options. For instance, you may wish to have your
program present the ``-R`` option to the user, let GMT handle the
parsing, and examine the values. You may also wish to encode your own
custom options that may require you to parse user text into the
corresponding floating point dimensions, constants, coordinates, absolute time, etc.
The API provides several functions to simplify these tedious parsing
tasks. This section is intended to show how the programmer will obtain
information from the user that is necessary to do the task at hand
(e.g., special options to provide values and settings for the program).
In the following section we will concern ourselves with preparing
arguments for calling any of the GMT modules.

Display usage syntax for GMT common options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can have your program menu display the standard usage message for a
GMT common option by calling the function

.. _GMT_Option:

  ::

    int GMT_Option (void *API, const char *options);

where ``options`` is a comma-separated list of GMT common options
(e.g., "R,J,O,X"). You can repeat this function with different sets of
options in order to intersperse your own custom options within an
overall alphabetical order; see any GMT module for examples of typical
layouts.

Parsing the GMT common options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parsing of all GMT common option is done by on call to

.. _GMT_Parse_Common:

  ::

    int GMT_Parse_Common (void *API, const char *args, struct GMT_OPTION *list);

where ``args`` is a string of the common GMT options your program is allowed to use.
An error will be reported if any of the common GMT options fail
to parse, and if so we return 1; if no errors we return 0. All
other options, including file names, will be silently ignored. The
parsing will update the internal GMT information structure that
affects module operations.

Inquiring about the GMT common options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The API provide only a limited window into the full GMT machinery
accessible to the modules. You can determine if a particular common
option has been parsed and in some cases determine the values that were set with

.. _GMT_Get_Common:

  ::

    int GMT_Get_Common (void *API, unsigned int option, double *par);

where ``option`` is a single option character (e.g., 'R') and ``par`` is
a double array with at least a length of 6. If the particular option has
been parsed then the function returns the number of parameters passed
back via ``par``; otherwise we return -1. For instance, to determine if
the ``-R`` was set and to obtain the specified region you may call

  ::

    if (GMT_Get_Common (API, 'R', wesn)) != -1) {
        /* wesn now contains the boundary information */
    }

The ``wesn`` array could now be passed to the various read and create
functions for GMT resources.

Parsing text values
~~~~~~~~~~~~~~~~~~~

Your program may need to request values from the user, such as
distances, plot dimensions, coordinates, date/time strings and other data. The conversion
from such text to actual distances, taking units into account, is
tedious to program. You can simplify this by using

.. _GMT_Get_Values:

  ::

    int GMT_Get_Values (void *API, const char *arg, double par[], int maxpar);

where ``arg`` is the text item with one or more values that are
separated by commas, spaces, tabs, semi-colons, or slashes, and ``par`` is an array of length ``maxpar`` long
enough to hold all the items you are parsing. The function returns the
number of items parsed with a maximum of ``maxpar``, or -1 if there is an error. For instance, assume
the character string ``origin`` was given by the user as two geographic
coordinates separated by a slash (e.g., ``"35:45W/19:30:55.3S"``). We
obtain the two coordinates in decimal degrees by calling

  ::

    n = GMT_Get_Values (API, origin, pair, 2);

Your program can now check that ``n`` equals 2 and then use the values
in ``pairs`` separately. **Note**: Dimensions given with units of inches, cm, or points
are converted to the current default unit set via :term:`PROJ_LENGTH_UNIT`,
while distances given in km, nautical miles, miles, feet, or
survey feet are returned in meters. Arc lengths in minutes and seconds
are returned in decimal degrees, and date/time values are returned in
seconds since the current epoch [1970].

Get or set an API or GMT default parameter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If your program needs to determine one or more of the current
API or GMT default settings you can do so via

.. _GMT_Get_Default:

  ::

    int GMT_Get_Default (void *API, const char *keyword, char *value);

where ``keyword`` is one such keyword (e.g., :term:`PROJ_LENGTH_UNIT`) and
``value`` must be a character string long enough to hold the answer.  In
addition to the long list of GMT defaults you can also inquire about the
API parameters ``API_PAD`` (the current pad setting), ``API_IMAGE_LAYOUT`` (the
order and structure of image memory storage), ``API_GRID_LAYOUT`` (order of
grid memory storage), ``API_VERSION`` (the API version string),
``API_CORES`` (the number of cores seen by the API),
``API_BINDIR`` (the API (GMT) executable path),
``API_SHAREDIR`` (the API (GMT) shared directory path),
``API_DATADIR`` (the API (GMT) data directory path), and
``API_PLUGINDIR`` (the API (GMT) plugin path).
Depending on what parameter you selected you could further convert it to
a numerical value with GMT_Get_Values_ or just use it in a text comparison.

To change any of the API or
GMT default settings programmatically you would use

.. _GMT_Set_Default:

  ::

    int GMT_Set_Default (void *API, const char *keyword, const char *value);

where as before ``keyword`` is one such keyword (e.g., :term:`PROJ_LENGTH_UNIT`) and
``value`` must be a character string with the new setting.
Note that all settings must be passed as text strings even if many are
inherently integers or floats.

Get an API enum constant
~~~~~~~~~~~~~~~~~~~~~~~~

The GMT API enum constants that are part of the API are defined in the
include file gmt_resources.h, which is included by gmt.h.  So, if you are
writing an application in C/C++ you are including gmt.h and thus have
access to all the API enums directly.  However, if your application is
written in other languages and you are perhaps just interfacing with the
shared GMT API library, then you can access any GMT enum via

.. _GMT_Get_Enum:

  ::

    int GMT_Get_Enum (void *API, const char *enumname);

where ``enumname`` is the name of one such enum (e.g., GMT_SESSION_EXTERNAL, GMT_IS_DATASET, etc.),
including the ones listed in :ref:`types <tbl-types>` and :ref:`types <tbl-viatypes>`; see
gmt_resources.h for the full listing.
The function returns the corresponding integer value.  For unrecognized names we return -99999.
**Note**: You may pass a NULL pointer as API if you need to obtain enum values prior to calling GMT_Create_Session_.

For indexed access to custom grids and images we may need to know the internal matrix layout.
You can change this information via

.. _GMT_Set_Index:

  ::

    int64_t GMT_Set_Index (struct GMT_GRID_HEADER *header, char *code);

where the ``header`` is the header of either a grid or image, and ``code`` is a three-character
code indication ...

.. _sec-func:

Call a module
-------------

One of the advantages of programming with the API is that you
have access to the high-level GMT modules. For example, if your
program must compute the distance from a node to all other nodes in the grid
then you can simply set up options and call :doc:`grdmath` to do it
for you and accept the result back as an input grid. All the module
interfaces are identical and are called via

.. _GMT_Call_Module:

  ::

    int GMT_Call_Module (void *API, const char *module, int mode, void *args);

Here, ``module`` is the name of any of the GMT modules, such as
:doc:`plot` or :doc:`grdvolume`.  All GMT modules may be called with one of
three sets of ``args`` depending on ``mode``. The three modes differ in
how the options are passed to the module:

    *mode* = ``GMT_MODULE_EXIST``.
        Return GMT_NOERROR (0) if the module exists, nonzero otherwise.

    *mode* = ``GMT_MODULE_PURPOSE``.
        Just print the one-line purpose of the module; args must be NULL.

    *mode* = ``GMT_MODULE_LIST``.
        Just prints a list of all modules (including those given as plugins); args must be NULL.

    *mode* = ``GMT_MODULE_OPT``.
        Expects ``args`` to be a pointer to a doubly-linked list of objects with individual
        options for the current program. We will see
        how API functions can help prepare and maintain such lists.

    *mode* = ``GMT_MODULE_CMD``.
        Expects ``args`` to be a single text string with all needed options.

    *mode > 0*.
        Expects ``args`` to be an array of text strings and ``mode`` to be a count of how many
        options are passed (i.e., the ``argc, argv[]`` model used by the GMT programs themselves).

From external interfaces and with a debug verbosity level set, ``GMT_Call_Module`` will
also print out the equivalent command line to standard error (or its substitute).

Set program options via text array arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When ``mode > 0`` we expect an array ``args`` of character
strings that each holds a single command line option (e.g.,
"-R120:30/134:45/8S/3N") and interpret ``mode`` to be the count of how
many options are passed. This, of course, is almost exactly how the
stand-alone GMT programs are called (and reflects how they themselves
are activated internally). We call this the "argc-argv" mode. Depending
on how your program obtains the necessary options you may find that this
interface offers all you need.

Set program options via text command
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If ``mode =`` 0 then ``args`` will be examined to see if it contains
several options within a single command string. If so we will break
these into separate options. This is useful if you wish to pass a single
string such as "-R120:30/134:45/8S/3N -JM6i mydata.txt -Sc0.2c". We call
this the "command" mode and it is extensively used by the modules themselves.

Set program options via linked structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The third, linked-list interface allows developers using higher-level
programming languages to pass all command options via a pointer to a
NULL-terminated, doubly-linked list of option structures, each
containing information about a single option. Here, instead of text
arguments we pass the pointer to the linked list of options mentioned
above, and ``mode`` must be passed as ``GMT_MODULE_OPT``. Using
this interface can be more involved since you need to generate the
linked list of program options; however, utility functions exist to
simplify its use. This interface is intended for programs whose internal
workings are better suited to generate such arguments -- we call this the
"options" mode. The order in the list is not important as GMT will
sort it internally according to need. The option structure is defined below.

.. _options:

  ::

    struct GMT_OPTION {
        char               option;  /* Single option character (e.g., 'G' for -G) */
        char              *arg;     /* String with arguments (NULL if not used) */
        struct GMT_OPTION *next;    /* Next option pointer (NULL for last option) */
        struct GMT_OPTION *prev;    /* Previous option (NULL for first option) */
    };

Convert between text and linked structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To assist programmers there are also two convenience functions that
allow you to convert between the two argument formats. They are

.. _GMT_Create_Options:

  ::

    struct GMT_OPTION *GMT_Create_Options (void *API, int argc, void *args);

This function accepts your array of text arguments (cast via a void
pointer), allocates the necessary space, performs the conversion, and
returns a pointer to the head of the linked list of program options.
However, in case of an error we return a NULL pointer and set
``API->error`` to indicate the nature of the problem. Otherwise, the
pointer may now be passed to the relevant GMT module. Note that if
your list of text arguments were obtained from a C ``main()`` function
then ``argv[0]`` will contain the name of the calling program. To avoid
passing this as a bad file name option, call GMT_Create_Options_ with
``argc-1`` and ``argv+1`` instead. If you wish to pass a single text string with
multiple options (in lieu of an array of text strings), then pass
``argc`` = 0. When no longer needed you can remove the entire list by calling

.. _GMT_Destroy_Options:

  ::

    int GMT_Destroy_Options (void *API, struct GMT_OPTION **list);

The function returns 1 if there is an error (which is passed back
with ``API->error``), otherwise it returns 0 (``GMT_NOERROR``).

The inverse function prototype is

.. _GMT_Create_Args:

  ::

    char **GMT_Create_Args (void *API, int *argc, struct GMT_OPTION *list);

which allocates space for the text strings and performs the conversion;
it passes back the count of the arguments via ``argc`` and returns a
pointer to the text array. In the case of an error we return a NULL
pointer and set ``API->error`` to reflect the error type. Note that
``argv[0]`` will not contain the name of the program as is the case the
arguments presented by a C ``main()`` function. When you no longer have
any use for the text array, call

.. _GMT_Destroy_Args:

  ::

    int GMT_Destroy_Args (void *API, int argc, char **argv[]);

to deallocate the space used. This function returns 1 if there is
an error (which is passed back with ``API->error``), otherwise it returns 0 (``GMT_NOERROR``).

Finally, to convert the linked list of option structures to a single
text string command, use

.. _GMT_Create_Cmd:

  ::

    char *GMT_Create_Cmd (void *API, struct GMT_OPTION *list);

Developers who plan to import and export GMT shell scripts might find
it convenient to use these functions. In case of an error we return a
NULL pointer and set ``API->error``, otherwise a pointer to an allocated
string is returned.  When you no longer have
any use for the text string, call

.. _GMT_Destroy_Cmd:

  ::

    int GMT_Destroy_Cmd (void *API, char **string);

to deallocate the space used. This function returns 1 if there is
an error (which is passed back with ``API->error``), otherwise it
returns 0  (``GMT_NOERROR``).

Manage the linked list of options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Several additional utility functions are available for programmers who
wish to manipulate program option structures within their own programs.
These allow you to create new option structures, append them to the
linked list, replace existing options with new values, find a particular
option, and remove options from the list. **Note**: The order in which the
options appear in the linked list is of no consequence to GMT.
Internally, GMT will sort and process the options in the manner
required. Externally, you are free to maintain your own order.

Make a new option structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^

GMT_Make_Option_ will allocate a new option structure, assign
values given the ``option`` and ``arg`` parameters (pass NULL if there is
no argument for this option), and return a pointer to the allocated
structure. The prototype is

.. _GMT_Make_Option:

  ::

    struct GMT_OPTION *GMT_Make_Option (void *API, char option, const char *arg);

Should memory allocation fail the function will print an error message
pass an error code via ``API->error``, and return NULL.

Append an option to the linked list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

GMT_Append_Option_ will append the specified ``option`` to the end of
the doubly-linked ``list``. The prototype is

.. _GMT_Append_Option:

  ::

    struct GMT_OPTION *GMT_Append_Option (void *API, struct GMT_OPTION *option,
    	struct GMT_OPTION *list);

We return the list back, and if ``list`` is given as NULL we return
``option`` as the start of the new list. Any errors result in a NULL
pointer with ``API->error`` holding the error type.

Find an option in the linked list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

GMT_Find_Option_ will return a pointer ``ptr`` to the first option in
the linked list starting at ``list`` whose option character equals
``option``. If not found we return NULL. While this is not necessarily
an error we still set ``API->error`` accordingly. The prototype is

.. _GMT_Find_Option:

  ::

    struct GMT_OPTION *GMT_Find_Option (void *API, char option,
    	struct GMT_OPTION *list);

If you need to look for multiple occurrences of a certain option you
will need to call GMT_Find_Option_ again, passing the option
following the previously found option as the ``list`` entry, i.e.,

  ::

    list = *ptr->next;

Update an existing option in the list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

GMT_Update_Option_ will replace the argument of ``current`` with the
new argument ``arg`` and otherwise leave the option at its place in the
list. The prototype is

.. _GMT_Update_Option:

  ::

    int GMT_Update_Option (void *API, struct GMT_OPTION *current, const char *arg);

An error will be reported if (a) ``current`` is NULL or (b) ``arg`` is
NULL. The function returns 1 if there is an error, otherwise it returns 0 (``GMT_NOERROR``).

Delete an existing option in the linked list
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You may use GMT_Delete_Option_ to remove the ``current`` option from the linked
``list``. The prototype is

.. _GMT_Delete_Option:

  ::

    int GMT_Delete_Option (void *API, struct GMT_OPTION *current, struct GMT_OPTION **head);

We return 1 if the option is not found in the list and set
``API->error`` accordingly. **Note**: Only the first occurrence of the
specified option will be deleted. If you need to delete all such options
you will need to call this function in a loop until it returns a
non-zero status.

Specify a file via a linked option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To specify an input file name via an option, simply use < as the
option (this is what GMT_Create_Options_ does when it finds filenames
on the command line). Likewise, > can be used to explicitly
indicate an output file. In order to append to an existing file, use
). For example the following command would read from file.A and
append to file.B:

  ::

    gmt convert -<file.A -)file.B

These options also work on the command line but usually one would have
to escape the special characters <, > and ) as they are normally
used for file redirection.

Encode option arguments for external interfaces
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Developers writing interfaces between GMT and external platforms such
as other languages (Python, Java, Julia, etc.) or tools (MATLAB, Octave,
etc.) need to manipulate linked options in a special way.  For instance,
a GMT call in the MATLAB or Octave application might look like

.. code-block:: none

    table = gmt('blockmean -R30W/30E/10S/10N -I2m', [x y z]);
    grid  = gmt('surface -R -I2m -Lu', table, high_limit_grid);
    grid2 = gmt('grdmath ? LOG10 ? MUL', grid, grid);

Most of the time our implicit rules will take care of the ordering.  The
rule says that all required input data items must be listed before any
secondary input data items, and all primary output items must be listed
on the left hand side before any secondary output items.
There are three situations where the parsing will need further help;
(1) Specifying the positions of memory arguments given to :doc:`gmtmath`,
(2) specifying the positions of memory arguments given to :doc:`grdmath`,
and (3) using -R? when passing a memory grid to the -R option (since just -R
means use the previous region in the command history).
Thus, in the :doc:`gmtmath` call we we needed to specify where
the specific arguments should be placed among the operators.
API developers will rely on GMT_Open_VirtualFile_ to convert the
above syntax to correct options for GMT_Call_Module_.
The prototype is

.. _GMT_Encode_Options:

  ::

    struct GMT_RESOURCE *GMT_Encode_Options (void *API, const char *module, int n_in,
    	                                       struct GMT_OPTION **head, int *n_items);

where ``module`` is the name of the module whose linked options are
pointed to by ``*head``, ``n_in`` contains the number of *input*
objects we have to connect (or -1 if not known) and we return an array
that contains specific information for those options that
(after processing) contain explicit memory references.  The number of
items in the array is returned via the ``n_items`` variable.  The function
returns NULL if there are errors and sets ``API->error`` to the corresponding
error number.  The GMT_RESOURCE structure is defined below:

.. .. _struct-grid:

.. code-block:: c

   struct GMT_RESOURCE {	/* Information for passing external resources */
       enum GMT_enum_family family;     /* GMT data family */
       enum GMT_enum_geometry geometry; /* One of the recognized GMT geometries */
       enum GMT_enum_std direction;     /* Either GMT_IN or GMT_OUT */
       struct GMT_OPTION *option;       /* Pointer to the corresponding module option */
       int object_ID;                   /* Object ID returned by GMT_Register_IO */
       int pos;                         /* Index into external object in|out arrays */
       int mode;                        /* 0 means primary i/o object, 1 means secondary */
       void *object;                    /* Pointer to the registered GMT object */
   };

API developers will need to provide specific code to handle the registration of native
structures in their language or application and to translate between the GMT resources
and the corresponding native items.  Developers should look at an existing and working
interface such as the GMT/MATLAB toolbox to see the required steps.

Expand an option with explicit memory references
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When the external tool or application knows the name of the special file names
used for memory references the developer should replace the place-holder ``?`` character
in any option string with the actual reference name.  This is accomplished by
calling GMT_Expand_Option_, with prototype

.. _GMT_Expand_Option:

  ::

    int GMT_Expand_Option (void *API, struct GMT_OPTION *option, const char *name);

where ``option`` is the current option and ``name``
is the special file name for the memory reference.

The GMT FFT Interface
=====================

While the i/o options presented so far lets you easily read in a data
table or grid and manipulate them, if you need to do the manipulation in the
wavenumber domain then this chapter is for you. Here, we outline how to
take the Fourier transform of such data, perform calculations in the
wavenumber domain, and take the inverse transform before writing the
results. To assist programmers we also distribute fully functioning
demonstration programs that takes you through the steps we are about to
discuss; these demo programs may be used as your starting point for
further development and can be found in the gmt-custom repository.

Presenting and parsing the FFT options
--------------------------------------

Several GMT programs that use the FFTs present the same unified option and
modifier sets to the user. The API makes these available as well. If
your program needs to present the FFT option usage you can call

.. _GMT_FFT_Option:

  ::

    unsigned int GMT_FFT_Option (void *API, char option, unsigned int dim,
    	                           const char *string);

Here, ``option`` is the unique character used for this particular
program option (most GMT programs have standardized on using 'N' but
you are free to choose whatever letter you want except existing GMT common
options). The ``dim`` sets the dimension of the transform; currently you
must choose 1 or 2, while ``string`` is a one-line message that
states what the option does; you should tailor this to your program. If
NULL then a generic message is placed instead.

To parse the user's selection you call

.. _GMT_FFT_Parse:

  ::

    void *GMT_FFT_Parse (void *API, char option, unsigned int dim, const char *arg);

which accepts the user's string option via ``arg``; the other arguments
are the same as those above. The function returns an opaque pointer to a
structure with the chosen parameters.

Initializing the FFT machinery
------------------------------

Before your can take any transforms you must initialize the FFT
machinery. This process involves a series of preparatory steps that are
conveniently performed for you by

.. _GMT_FFT_Create:

  ::

    void *GMT_FFT_Create (void *API, void *X, unsigned int dim,
    	                    unsigned int mode, void *F);

Here, ``X`` is either your dataset or grid pointer, ``dim`` is the
dimension of the transform (1 or 2 only), ``mode`` passes various flags to the setup, such as whether
the data is real, imaginary, or complex, and ``F`` is the opaque pointer
previously returned by GMT_FFT_Parse_. Depending on the option string you passed to
GMT_FFT_Parse_, the data may have a constant level or a trend
removed, mirror reflected and extended by various symmetries, padded and
tapered to desired transform dimensions, and possibly
temporary files are written out before the transform takes place. See the :doc:`grdfft`
man page for a full explanation of the options presented by GMT_FFT_Option_.

Taking the FFT
--------------

Now that everything has been set up you can perform the transform with

.. _GMT_FFT:

  ::

    void *GMT_FFT (void *API, void *X, int direction, unsigned int mode, void *K);

which takes as ``direction`` either ``GMT_FFT_FWD`` or ``GMT_FFT_INV``. The
``mode`` is used to specify if we pass a real (``GMT_FFT_REAL``) or complex
(``GMT_FFT_COMPLEX``) data set, and ``K`` is the opaque pointer returned
by GMT_FFT_Create_. The transform is performed in place and returned
via ``X``. When done with your manipulations (below) you can call it
again with the inverse direction to recover the corresponding space-domain
version of your data. The FFT is fully normalized so that calling
forward followed by inverse yields the original data set. The information
passed via ``K`` determines if a 1-D or 2-D transform takes place; the
key work is done via ``GMT_FFT_1D`` or ``GMT_FFT_2D``, as explained below.

Taking the 1-D FFT
------------------

A lower-level 1-D FFT is also available via the API, i.e.,

.. _GMT_FFT_1D:

  ::

    int GMT_FFT_1D (void *API, float *data, uint64_t n, int direction,
    	unsigned int mode);

which takes as ``direction`` either ``GMT_FFT_FWD`` or ``GMT_FFT_INV``. The
``mode`` is used to specify if we pass a real (``GMT_FFT_REAL``) or complex
(``GMT_FFT_COMPLEX``) data set, and ``data`` is the 1-D data array of length
``n`` that we wish
to transform. The transform is performed in place and returned
via ``data``. When done with your manipulations (below) you can call it
again with the inverse direction to recover the corresponding space-domain
version of your data. The 1-D FFT is fully normalized so that calling
forward followed by inverse yields the original data set.

Taking the 2-D FFT
------------------

A lower-level 2-D FFT is also available via

.. _GMT_FFT_2D:

  ::

    int GMT_FFT_2D (void *API, float *data, unsigned int n_columns,
    	              unsigned int n_rows, int direction, unsigned int mode);

which takes as ``direction`` either ``GMT_FFT_FWD`` or ``GMT_FFT_INV``. The
``mode`` is used to specify if we pass a real (``GMT_FFT_REAL``) or complex
(``GMT_FFT_COMPLEX``) data set, and ``data`` is the 2-D data array in
row-major format, with row length ``n_columns`` and column length ``n_rows``.
The transform is performed in place and returned
via ``data``. When done with your manipulations (below) you can call it
again with the inverse direction to recover the corresponding space-domain
version of your data. The 2-D FFT is fully normalized so that calling
forward followed by inverse yields the original data set.

Wavenumber calculations
-----------------------

As your data have been transformed to the wavenumber domain you may wish
to operate on the various values as a function of wavenumber. We will
show how this is done for datasets and grids separately. First, we
present the function that returns an individual wavenumber:

.. _GMT_FFT_Wavenumber:

  ::

    double GMT_FFT_Wavenumber (void *API, uint64_t k, unsigned int mode, void *K);

where ``k`` is the index into the array or grid, ``mode`` specifies
which wavenumber we want (it is not used for 1-D transform but for the
2-D transform we can select either the x-wavenumber (0), the
y-wavenumber (1), or the radial wavenumber (2)), and finally the opaque
vector created by GMT_FFT_Create_.

1-D FFT manipulation
~~~~~~~~~~~~~~~~~~~~

[To be added after gmtfft has been added as new module, probably in 5.4.]

2-D FFT manipulation
~~~~~~~~~~~~~~~~~~~~

The number of complex pairs in the grid is given by the header's ``nm``
variable, while ``size`` will be twice that value as it holds the number
of components. To visit all the complex values and obtain the
corresponding wavenumber we simply need to loop over ``size`` and call
GMT_FFT_Wavenumber_. This code snippet multiples the complex grid by
the radial wavenumber:

  ::

    uint64_t k;
    for (k = 0; k < Grid->header->size; k++) {
        wave = GMT_FFT_Wavenumber (API, k, 2, K);
        Grid->data[k] *= wave;
    }

Alternatively, you may choose to be more specific about which components
are real and imaginary (especially if they are to be treated
differently), and set up the loop this way:

  ::

    uint64_t re, im;
    for (re = 0, im = 1; re < Grid->header->size; re += 2, im += 2) {
        wave = GMT_FFT_Wavenumber (API, re, 2, K);
        Grid->data[re] *= wave;
        Grid->data[im] *= 2.0 * wave;
    }

Destroying the FFT machinery
----------------------------

When done you terminate the FFT machinery with

.. _GMT_FFT_Destroy:

  ::

    double GMT_FFT_Destroy (void *API, void *K);

which simply frees up the memory allocated by the FFT machinery with GMT_FFT_Create_.

FORTRAN Support
===============

FORTRAN 90 developers who wish to use the GMT API may use the same
API functions as discussed in Chapter 2. As we do not have much (i.e., any) experience
with modern Fortran we are not sure to what extent you are able to access
the members of the various structures, such as the :ref:`GMT_GRID <struct-grid>` structure. Thus,
this part will depend on feedback and for the time being is to be considered
preliminary and subject to change.  We encourage you to take contact should you
wish to use the API with your Fortran 90 programs.

FORTRAN 77 Grid i/o
-------------------

Because of a lack of structure pointers we can only provide a low level of
support for Fortran 77. This API is limited to help you inquire, read and write
GMT grids directly from Fortran 77.
To inquire about the range of information in a grid, use

.. _gmt_f77_readgrdinfo:

  ::

    int gmt_f77_readgrdinfo (unsigned int dim[], double limits[], double inc[],
    	                       char *title, char *remark, const char *file)

where ``dim`` returns the grid width, height, and registration, ``limits`` returns the min and max values for x, y, and z
as three consecutive pairs, ``inc`` returns the x and y increments, while the ``title`` and ``remark``
return the values of these strings. The ``file``
argument is the name of the file we wish to inquire about. The function returns 0 unless there is an error.
Note that you must declare your variables so that ``limits`` has at least 6 elements, ``inc`` has at least 2, and ``dim`` has at least 4.

To actually read the grid, we use

.. _gmt_f77_readgrd:

  ::

    int gmt_f77_readgrd (float *array, unsigned int dim[], double wesn[],
    	                   double inc[], char *title, char *remark, const char *file)

where ``array`` is the 1-D grid data array, ``dim`` returns the grid width, height, and registration,
``limits`` returns the min and max values for x, y, and z, ``inc`` returns the x and y increments, and
the ``title`` and ``remark`` return the values of the corresponding strings.  The ``file``
argument is the name of the file we wish to read from.  The function returns 0 unless there is an error.
Note on input, ``dim[2]`` can be set to 1, which means we will allocate the array for you; otherwise
we assume space has already been secured.  Also, if ``dim[3]`` is set to 1 we will in-place transpose
the array from C-style row-major array order to Fortran column-major array order.

Finally, to write a grid to file you can use

.. _gmt_f77_writegrd:

  ::

    int gmt_f77_writegrd_(float *array, unsigned int dim[], double wesn[], double inc[],
    	                    const char *title, const char *remark, const char *file)

where ``array`` is the 1-D grid data array, ``dim`` specifies the grid width, height, and registration,
``limits`` may be used to specify a subset (normally, just pass zeros), ``inc`` specifies the x and y increments,
while the ``title`` and ``remark`` supply the values of these strings.  The ``file``
argument is the name of the file we wish to write to.  The function returns 0 unless there is an error.
If ``dim[3]`` is set to 1 we will in-place transpose
the array from Fortran column-major array order to C-style row-major array order before writing. Note
this means ``array`` will have been transposed when the function returns.

External Interfaces
===================

Developers may want to access GMT modules from external programming environments, such as MATLAB,
Octave, Julia, Python, R, IDL, etc., etc.  These all face similar challenges and hence this section
will speak in somewhat abstract terms.  Specific language addressing the challenges for some of
the above-mentioned environments will follow below.

The C/C++ API for GMT makes it possible to call any of the ~100 core modules, the 40 or so supplemental
modules, and any number of custom modules provided via shared libraries (e.g., the gsfml modules).  Many
of the external interfaces come equipped with methods to call C functions directly.
The key challenges pertain to specifying the input to use in the module and to receive
what is produced by the module.
As we know from GMT command line usage, all GMT modules expect input to be given via input files (or stdin, except for sources like grids and images).  Similarly, output will be written to a specified
output file (or stdout if the data type supports it).  Clearly, external interfaces
could do the same thing.  The problem is that most of the time we already will have the input data in
memory and would prefer the output to be returned back to memory, thus avoiding using temporary files.
Here, we will outline the general approach for using the GMT API.  We will describe a relatively low-level approach
to calling GMT modules.  Once such an interface exists it is simpler to build a more flexible and user-friendly
layer on top that can handle argument parsing in a form that makes the interface seem more of a natural
extension of your external environment than a forced fit to GMT's command-line heritage.
Before we describe the interface it is important to understand that the GMT modules, since the beginning
or time, have done the i/o inside the modules.  While these steps are helped by i/o library functions, the
i/o activities all take place *inside* the modules.  This means that external environments in which the desired
input data already reside in memory and the desired results should be returned back to memory pose a
trickier challenge.  We will see the solution to this involves the concept of *virtual* files.

.. figure:: /_images/GMT_API_use.*
   :width: 500 px
   :align: center

   GMT Modules can read and write information in may ways.  The GMT command line modules
   can only access the methods in white, while all methods are available via the C API.
   External interfaces will preferentially want the methods in orange.

Plain interface
---------------

While the syntax of your external environment's language will dictate the details of the implementation, we will in general
need to build a function (or class, or method) that allows you to issue a call like this:

[*results*] = **gmt** (*module*, *options*, *inputs*)

where *results* (i.e., objects returned back to memory) is optional and may be one or more items grouped
together, depending on language syntax.  If no output is required then no left-hand side
assignment will be present.  Likewise, *inputs* is optional and may be one or more comma-separated
objects present in memory.  In most cases, *options* will be required and this is a string with
options very similar to the arguments given on the GMT command line.  Finally, *module* is required since you
must specify which one you want to call. The coding of the **gmt** method, class, or function above may be written entirely in
C, partly in C and the external scripting language, or entirely in the scripting language, depending on
restrictions on what needs to be done and where this is most easily accomplished.
How this is accomplished may vary from environment to environment.

.. figure:: /_images/GMT_API_flow.*
   :width: 500 px
   :align: center

   Data pass in and out of the **gmt** interface which may be written in the scripting language used
   by the external interface.  The native data will need to be encapsulated by GMT containers and this
   step may be done by a C **parser** but could also be done by the **gmt** interface directly.  Either
   of these communicate directly with the C functions in the GMT API.

Data containers
---------------

The external interface developer will need to create native data classes or structures that are capable of
containing the information associated with the six GMT objects: data tables, grids, images, cubes, color palette tables,
and PostScript documents.  In other words, how your external environment will represent these
data in memory.  Some of these "containers" may already exist, while others may need to be designed.  Most likely, you will end up with
a set of six containers that can hold the various GMT data objects and related metadata.  In addition, it may
be convenient to also consider the two GMT helper objects MATRIX and VECTOR, which may be closer to the native
representation of your data than, for instance, the native GMT_DATASET.

Input from memory
-----------------

Whether input comes from memory or from external files, the call to a GMT module is the same: we have to specify
*filenames* to provide the input data.  Thus, the game is to provide *virtual* file names that represent our in-memory
data.  The process is relatively simple and may need to be done in a snippet of C
code that can be called by a function written in your environments scripting language. The steps go like this:

#. Create a GMT C container marked for input and copy or reference your data provided by
   your external environment into this container.
#. Open a virtual file using this container to represent the input source.
#. Insert this virtual file name in the appropriate location in the GMT option string.  If the
   module imports data from *stdin* then we can use the hidden option -<filename.

When the GMT module is run it will know how to make the connections between the virtual file names and
the actual data via information stored inside the C API.  When the module completes you should close any
open virtual files that were used by the module.

Output to memory
----------------

As the case for selecting input, GMT modules only know about writing results to a file (or stdout).  Hence, we must follow the same paradigm as we did for input
and identify virtual files to represent the output destinations.  The steps are:

#. Create an empty GMT C container of the right type marked for output.
#. Create a virtual file name to represent this output destination.
#. Place this file name in the appropriate location in the GMT option string.  If the
   module exports data to *stdout* then we can use the hidden option ->filename.

When the GMT module is run it will know how to make the connections between the memory allocated by the
module and the virtual file names stored inside the C API.  Once the module call has completed you can access the
results in the external environment by using GMT_Read_VirtualFile_ with the virtual filename you created earlier.  This will return a GMT C container with the results, and
you can now populate you external data containers with data produced by the GMT module.

The magic of knowing
--------------------

External developers have access to the two extra API functions GMT_Encode_Options_ and GMT_Expand_Option_.
Your **gmt** will need to call GMT_Encode_Options_ to obtain information about what the selected
module expects, what its options are, which were selected, and what data types are expected.  It may
possibly modify the options, such as adding the filename "?" to options that set
*required* input and output files and returns an array of structures with specific information about
all inputs and outputs.  If sources and destinations were missing from your *options* string it is taken
to mean that you want to associate these sources and destinations
with memory locations rather than actual files.  The second function GMT_Expand_Option_ can then then
used to replace these place-holder names with the virtual filenames you created earlier.

The MATLAB interface
~~~~~~~~~~~~~~~~~~~~

We have built a MATLAB/Octave interface to GMT called the toolbox.  It was our first attempt to use the C API from an
external environment and its development influenced
how we designed the final GMT C API.  MATLAB represents most data as matrices but there are also structures that
can hold many different items, including several matrices and text strings.  Thus, we designed several native mex structures
that represent the six GMT objects.  The main **gmt** function available in MATLAB derives from a small MATLAB script
(gmt.m) which handles basic argument testing and then passes the arguments to our C function gmtmex.c.
Most of the high-level parsing of options and arguments is done in this function, but we also rely on
a C library (gmtmex_parser.c) that hides the details of the implementation.  It is this library that
does most of the work in translating between the GMT and MATLAB object layouts.  Knowing what types are
represented by the different sources and destinations is provided by the array of structures returned
by GMT_Encode_Options_.

The Julia interface
~~~~~~~~~~~~~~~~~~~

Unlike the MATLAB interface, the Julia interface GMT.jl is written entirely in the Julia language.

The Python interface
~~~~~~~~~~~~~~~~~~~~

Unlike the MATLAB interface, the Python interface PyGMT is written entirely in the Python language.

Appendix A: GMT resources
-------------------------

We earlier introduced the six standard GMT resources (dataset, grid, image, cube, color palette table, PostScript)
as well as the user vector and matrix.  Here are the complete definitions of these structures, including
all variables accessible via the structures.

Data set
~~~~~~~~

Each data set is represented by a :ref:`GMT_DATASET <struct-dataset>` that consists of one or more data
tables represented by a :ref:`GMT_DATATABLE <struct-datatable>`, and each table consists of one or more
segments represented by a :ref:`GMT_DATASEGMENT <struct-datasegment>`, and each segment contains one or
more rows of a fixed number of columns.

.. _struct-dataset:

.. code-block:: c

   struct GMT_DATASET {	/* Single container for an array of GMT tables (files) */
       /* Variables we document for the API: */
       uint64_t               n_tables;       /* Total number of tables (files) contained */
       uint64_t               n_columns;      /* Number of data columns */
       uint64_t               n_segments;     /* Total number of segments across all tables */
       uint64_t               n_records;      /* Total number of data records across all tables */
       double                 *min;           /* Minimum coordinate for each column */
       double                 *max;           /* Maximum coordinate for each column */
       struct GMT_DATATABLE   **table;        /* Pointer to array of tables */
       unsigned int           type;           /* The data record type of this dataset */
       unsigned int           geometry;       /* The geometry of this dataset */
       const char             *ProjRefPROJ4;  /* To store a referencing system string in PROJ.4 format */
       const char             *ProjRefWKT;    /* To store a referencing system string in WKT format */
       int                    ProjRefEPSG;    /* To store a referencing system EPSG code */
       void                   *hidden;        /* ---- Variables "hidden" from the API ---- */
   };

Here is the full definition of the ``GMT_DATATABLE`` structure:

.. _struct-datatable:

.. code-block:: c

   struct GMT_DATATABLE {  /* To hold an array of line segment structures and header information in one container */
       /* Variables we document for the API: */
       unsigned int n_headers;           /* Number of file header records (0 if no header) */
       uint64_t n_columns;               /* Number of columns (fields) in each record */
       uint64_t n_segments;              /* Number of segments in the array */
       uint64_t n_records;               /* Total number of data records across all segments */
       double *min;                      /* Minimum coordinate for each column */
       double *max;                      /* Maximum coordinate for each column */
       char **header;                    /* Array with all file header records, if any) */
       struct GMT_DATASEGMENT **segment; /* Pointer to array of segments */
       void *hidden;                     /* ---- Variables "hidden" from the API ---- */
   };

Here is the full definition of the ``GMT_DATASEGMENT`` structure:

.. _struct-datasegment:

.. code-block:: c

   struct GMT_DATASEGMENT {     /* For holding segment lines in memory */
       /* Variables we document for the API: */
       uint64_t n_rows;         /* Number of points in this segment */
       uint64_t n_columns;      /* Number of fields in each record (>= 2) */
       double *min;             /* Minimum coordinate for each column */
       double *max;             /* Maximum coordinate for each column */
       double **data;           /* Data x,y, and possibly other columns */
       char **text;             /* trailing text strings beyond the data */
       char *label;             /* Label string (if applicable) */
       char *header;            /* Segment header (if applicable) */
       void *hidden;            /* ---- Variables "hidden" from the API ---- */
    };

GMT grid
~~~~~~~~

A grid is represented by a :ref:`GMT_GRID <struct-grid>` that consists of a header structure
represented by a :ref:`GMT_GRID_HEADER <struct-gridheader>` and an float array ``data`` that
contains the grid values.

.. _struct-grid:

.. code-block:: c

   struct GMT_GRID {                        /* To hold a GMT float grid and its header in one container */
       struct GMT_GRID_HEADER *header;      /* Pointer to full GMT header for the grid */
       float                  *data;        /* Pointer to the float grid */
       double                 *x, *y;       /* Vector of coordinates */
       void *hidden;                        /* ---- Variables "hidden" from the API ---- */
   };

The full definition of the ``GMT_GRID_HEADER`` structure.  Most of these members are only used internally:

.. _struct-gridheader:

.. code-block:: c

   struct GMT_GRID_HEADER {
       /* Variables we document for the API:
          They are copied verbatim to the native grid header and must be 4-byte unsigned ints. */
       uint32_t n_columns;                   /* Number of columns */
       uint32_t n_rows;                      /* Number of rows */
       uint32_t registration;                /* GMT_GRID_NODE_REG (0) or GMT_GRID_PIXEL_REG (1) */

       /* == The types of the following 12 elements must not be changed.
          == They are also copied verbatim to the native grid header. */
       double wesn[4];                         /* Min/max x and y coordinates */
       double z_min;                           /* Minimum z value */
       double z_max;                           /* Maximum z value */
       double inc[2];                          /* x and y increment */
       double z_scale_factor;                  /* grd values must be multiplied by this */
       double z_add_offset;                    /* After scaling, add this */
       char   x_units[GMT_GRID_UNIT_LEN80];    /* units in x-direction */
       char   y_units[GMT_GRID_UNIT_LEN80];    /* units in y-direction */
       char   z_units[GMT_GRID_UNIT_LEN80];    /* grid value units */
       char   title[GMT_GRID_TITLE_LEN80];     /* name of data set */
       char   command[GMT_GRID_COMMAND_LEN320];/* name of generating command */
       char   remark[GMT_GRID_REMARK_LEN160];  /* comments re this data set */
       /* == End of "untouchable" header.       */

       /* This section is flexible.  It is not copied to any grid header
          or stored in any file. It is considered private */
       unsigned int type;               /* Grid format */
       unsigned int bits;               /* Bits per value (e.g., 32 for ints/floats; 8 for bytes) */
       unsigned int complex_mode;       /* 0 = normal, GMT_GRID_IS_COMPLEX_REAL = real part of complex
										                       grid, GMT_GRID_IS_COMPLEX_IMAG = imag part of complex grid */
       unsigned int mx, my;             /* Actual dimensions of the grid in memory, allowing for the padding */
       size_t       nm;                 /* Number of data items in this grid (n_columns * n_rows) [padding is excluded] */
       size_t       size;               /* Actual number of items (not bytes) required to hold this grid (= mx * my), per band */
       size_t       n_alloc;            /* Bytes allocated for this grid */
       unsigned int n_bands;            /* Number of bands [1]. Used with IMAGE containers and macros to get ij index from row,col, band */
       unsigned int pad[4];             /* Padding on west, east, south, north sides [2,2,2,2] */
       const char  *ProjRefPROJ4;       /* To store a referencing system string in PROJ.4 format */
       const char  *ProjRefWKT;         /* To store a referencing system string in WKT format */
       float        nan_value;          /* Missing value as stored in grid file */
       double       xy_off;             /* 0.0 (registration == GMT_GRID_NODE_REG) or 0.5 ( == GMT_GRID_PIXEL_REG) */
       void        *hidden;             /* ---- Variables "hidden" from the API ---- */
   };

GMT image
~~~~~~~~~

An image is similar to a grid except it may have more than one layer (i.e., band).
It is represented by a :ref:`GMT_IMAGE <struct-image>` structure that consists of the
:ref:`GMT_GRID_HEADER <struct-gridheader>` structure and an char array ``data`` that
contains the image values.  The type of the array is determined by the value of ``type``.
**Note**: The header *size* value reflects number of nodes per band, so the actual memory
allocated will be *size * n_bands*.

.. _struct-image:

.. code-block:: c

  struct GMT_IMAGE {
      enum GMT_enum_type      type;             /* Data type, e.g. GMT_FLOAT */
      int                    *colormap;         /* Array with color lookup values */
      int                     n_indexed_colors; /* Number of colors in a color-mapped image */
      struct GMT_GRID_HEADER *header;           /* Pointer to full GMT header for the image */
      unsigned char          *data;             /* Pointer to actual image */
      unsigned char          *alpha;            /* Pointer to an optional transparency layer */
      const char             *color_interp;	/* Color interpretation name */
      double                 *x, *y;            /* Vector of coordinates */
      void                   *hidden;           /* ---- Variables "hidden" from the API ---- */
  };

GMT cube
~~~~~~~~

A 3-D cube is similar to a grid but typically has more than one layer.
It is represented by a :ref:`GMT_CUBE <struct-cube>` structure that consists of the
:ref:`GMT_GRID_HEADER <struct-gridheader>` structure and an float array ``data`` that
contains the cube values.
**Note**: The header *size* value reflects number of nodes per layer, so the actual memory
allocated will be *size * n_bands*, where the latter is one of the parameters in the header.

.. _struct-cube:

.. code-block:: c

  struct GMT_CUBE {
       struct GMT_GRID_HEADER *header;      /* The full GMT header for the grid */
       float                  *data;        /* Pointer to the float 3-D array */
       unsigned int           mode;         /* Indicates data originated as a list of 2-D grids rather than a cube */
       double                 z_range[2];   /* Minimum/max z values (complements header->wesn) */
       double                 z_inc;        /* z increment (complements header->inc) (0 if variable z spacing) */
       double                 *x, *y, *z;   /* Arrays of x,y,z coordinates */
       char name[GMT_GRID_UNIT_LEN80];      /* Name of variable, if read from file (empty if default) */
       char units[GMT_GRID_UNIT_LEN80];     /* Units in 3rd direction (complements x_units, y_units, z_units)  */
       void                   *hidden;      /* ---- Variables "hidden" from the API ---- */
 };

CPT palette table
~~~~~~~~~~~~~~~~~

A CPT is represented by a :ref:`GMT_PALETTE <struct-palette>` structure that contains several
items, such as a :ref:`GMT_LUT <struct-lut>` structure ``data`` that
contains the color information per interval.  The background, foreground and Nan-color values have
colors specified by the :ref:`GMT_BFN <struct-bnf>` array structure ``bfn``.  As each actual
color may be specified in different ways, including as an image, each color slice is represented by
the :ref:`GMT_FILL <struct-fill>` structure.

.. _struct-palette:

.. code-block:: c

   struct GMT_PALETTE {		/* Holds all pen, color, and fill-related parameters */
       /* Variables we document for the API: */
       struct GMT_LUT       *data;               /* CPT lookup data read by GMT_read_cpt */
       struct GMT_BFN        bfn[3];             /* Structures with back/fore/nan fills */
       unsigned int          n_headers;          /* Number of CPT header records (0 if no header) */
       unsigned int          n_colors;           /* Number of colors in CPT lookup table */
       unsigned int          mode;               /* Flags controlling use of BFN colors */
       unsigned int          model;              /* RGB, HSV, CMYK */
       unsigned int          is_wrapping;        /* true if a cyclic colortable */
       unsigned int          is_gray;            /* true if only grayshades are needed */
       unsigned int          is_bw;              /* true if only black and white are needed */
       unsigned int          is_continuous;      /* true if continuous color tables have been given */
       unsigned int          has_pattern;        /* true if CPT contains any patterns */
       unsigned int          has_hinge;          /* true if CPT has a hinge */
       unsigned int          has_range;          /* true if CPT has a natural range */
       unsigned int          categorical;        /* true if CPT applies to categorical data */
       double                minmax[2];          /* The default range, if has_range is true */
       double                hinge;              /* The default hinge, if is_wrapping is true */
       double                wrap_length;        /* The default period, if has_hinge is true */
       char                **header;             /* Array with all CPT header records, if any) */
       void                 *hidden;             /* ---- Variables "hidden" from the API ---- */
   };

The full definition of the ``GMT_LUT`` structure.

.. _struct-lut:

.. code-block:: c

   struct GMT_LUT {         /* For back-, fore-, and nan-colors */
       double                z_low, z_high, i_dz;
       double                rgb_low[4], rgb_high[4], rgb_diff[4];
       double                hsv_low[4], hsv_high[4], hsv_diff[4];
       unsigned int          annot;              /* 1 for Lower, 2 for Upper, 3 for Both */
       unsigned int          skip;               /* true means skip this slice */
       struct GMT_FILL      *fill;               /* For patterns instead of color */
       char                 *label;              /* For non-number labels */
   };

The full definition of the ``GMT_BFN`` structure:

.. _struct-bnf:

.. code-block:: c

   struct GMT_BFN {   /* For back-, fore-, and nan-colors */
       double                rgb[4];             /* Red, green, blue, and alpha */
       double                hsv[4];             /* Hue, saturation, value, alpha */
       unsigned int          skip;               /* true means skip this slice */
       struct GMT_FILL      *fill;               /* For patterns instead of color */
   };

The full definition of the ``GMT_FILL`` structure.  **Note**: Not part of the GMT API:

.. _struct-fill:

.. code-block:: c

   struct GMT_FILL {        /*! Holds fill attributes */
       double                rgb[4];             /* Chosen color if no pattern + Transparency 0-1 [0 = opaque] */
       double                f_rgb[4], b_rgb[4]; /* Colors applied to unset and set bits in 1-bit image */
       bool                  use_pattern;        /* true if pattern rather than rgb is set */
       int                   pattern_no;         /* Number of a predefined pattern, or -1 if not set */
       unsigned int          dpi;                /* Desired dpi of image building-block if use_pattern is true */
       char                  pattern[GMT_BUFSIZ];/* Full filename of user-defined raster pattern */
   };


PostScript text
~~~~~~~~~~~~~~~

Bulk PostScript is represented by a :ref:`GMT_POSTSCRIPT <struct-postscript>` structure that contains
``data`` that points to the text array containing ``n_bytes`` characters of raw PostScript code.  The
``mode`` parameter reflects the status of the PostScript document.

.. _struct-postscript:

.. code-block:: c

   struct GMT_POSTSCRIPT {	/* Single container for a chunk of PostScript code */
       /* Variables we document for the API: */
       unsigned int n_headers;          /* Number of PostScript header records (0 if no header) */
       size_t n_bytes;                  /* Length of data array so far */
       unsigned int mode;               /* Bit-flag for header (1) and trailer (2) */
       char *data;                      /* Pointer to PostScript code */
       char **header;                   /* Array with all PostScript header records, if any) */
       void *hidden;                    /* ---- Variables "hidden" from the API ---- */
   };

Matrix
~~~~~~

User matrices are represented by a :ref:`GMT_MATRIX <struct-matrix>` structure that contains
``data`` that points to an array of size ``n_columns`` by ``n_rows``.  The
``type`` indicates the memory type of the matrix, which is represented
by the :ref:`GMT_UNIVECTOR <struct-univector>` union.

.. _struct-matrix:

.. code-block:: c

  struct GMT_MATRIX {
      uint64_t             n_rows;        /* Number of rows in the matrix */
      uint64_t             n_columns;     /* Number of columns in the matrix */
      uint64_t             n_layers;      /* Number of layers in a 3-D matrix */
      enum GMT_enum_fmt    shape;         /* 0 = C (rows) and 1 = Fortran (cols) */
      enum GMT_enum_reg    registration;  /* 0 for gridline and 1 for pixel registration  */
      size_t               dim;           /* Allocated length of longest C or Fortran dim */
      size_t               size;          /* Byte length of data */
      enum GMT_enum_type   type;          /* Data type, e.g. GMT_FLOAT */
      double               range[6];      /* Contains xmin/xmax/ymin/ymax[/zmin/zmax] */
      union GMT_UNIVECTOR  data;          /* Union with pointer to actual matrix of the chosen type */
      char               **text;          /* Pointer to optional array of strings [NULL] */
      char               **header;        /* Array with all Vector header records, if any) */
      char command[GMT_GRID_COMMAND_LEN320]; /* name of generating command */
      char remark[GMT_GRID_REMARK_LEN160];   /* comments re this data set */
      const char          *ProjRefPROJ4;  /* To store a referencing system string in PROJ.4 format */
      const char          *ProjRefWKT;    /* To store a referencing system string in WKT format */
      int                  ProjRefEPSG;   /* To store a referencing system EPSG code */
      void                *hidden;        /* ---- Variables "hidden" from the API ---- */
  };

Vectors
~~~~~~~

User vectors are represented by a :ref:`GMT_VECTOR <struct-vector>` structure that contains
``data`` that points to an array of ``n_columns`` individual vectors.  The
``type`` array indicates the memory type of each vector.  Each vector is represented
by the :ref:`GMT_UNIVECTOR <struct-univector>` union which can accommodate any data type.

.. _struct-vector:

.. code-block:: c

  struct GMT_VECTOR {
      uint64_t             n_columns;     /* Number of vectors */
      uint64_t             n_rows;        /* Number of rows in each vector */
      enum GMT_enum_reg    registration;  /* 0 for gridline and 1 for pixel registration */
      enum GMT_enum_type  *type;          /* Array with data type for each vector */
      union GMT_UNIVECTOR *data;          /* Array with unions for each column */
      double               range[2];      /* The min and max limits on t-range (or 0,0) */
      char               **text;          /* Pointer to optional array of strings [NULL] */
      char               **header;        /* Array with all Vector header records, if any) */
      char command[GMT_GRID_COMMAND_LEN320]; /* name of generating command */
      char remark[GMT_GRID_REMARK_LEN160];   /* comments re this data set */
      const char          *ProjRefPROJ4;  /* To store a referencing system string in PROJ.4 format */
      const char          *ProjRefWKT;    /* To store a referencing system string in WKT format */
      int                  ProjRefEPSG;   /* To store a referencing system EPSG code */
      void                *hidden;        /* ---- Variables "hidden" from the API ---- */
  };

The full definition of the ``GMT_UNIVECTOR`` union that holds a pointer to any array or matrix type:

.. _struct-univector:

.. code-block:: c

  union GMT_UNIVECTOR {
      uint8_t  *uc1;       /* Pointer for unsigned 1-byte array */
      int8_t   *sc1;       /* Pointer for signed 1-byte array */
      uint16_t *ui2;       /* Pointer for unsigned 2-byte array */
      int16_t  *si2;       /* Pointer for signed 2-byte array */
      uint32_t *ui4;       /* Pointer for unsigned 4-byte array */
      int32_t  *si4;       /* Pointer for signed 4-byte array */
      uint64_t *ui8;       /* Pointer for unsigned 8-byte array */
      int64_t  *si8;       /* Pointer for signed 8-byte array */
      float    *f4;        /* Pointer for float array */
      double   *f8;        /* Pointer for double array */
  };


Appendix B: GMT constants
-------------------------

To increase readability we have encoded many simple integer constants as named
enum.  These are listed in the tables below and used as flags to various API
functions.

.. _tbl-types:

    +--------------+------------------------------------------+
    | constant     | description                              |
    +==============+==========================================+
    | GMT_CHAR     | int8_t, 1-byte signed integer type       |
    +--------------+------------------------------------------+
    | GMT_UCHAR    | int8_t, 1-byte unsigned integer type     |
    +--------------+------------------------------------------+
    | GMT_SHORT    | int16_t, 2-byte signed integer type      |
    +--------------+------------------------------------------+
    | GMT_USHORT   | uint16_t, 2-byte unsigned integer type   |
    +--------------+------------------------------------------+
    | GMT_INT      | int32_t, 4-byte signed integer type      |
    +--------------+------------------------------------------+
    | GMT_UINT     | uint32_t, 4-byte unsigned integer type   |
    +--------------+------------------------------------------+
    | GMT_LONG     | int64_t, 8-byte signed integer type      |
    +--------------+------------------------------------------+
    | GMT_ULONG    | uint64_t, 8-byte unsigned integer type   |
    +--------------+------------------------------------------+
    | GMT_FLOAT    | 4-byte data float type                   |
    +--------------+------------------------------------------+
    | GMT_DOUBLE   | 8-byte data float type                   |
    +--------------+------------------------------------------+

    The known data types in the GMT API.

When GMT_Open_VirtualFile_ is used with a NULL pointer to create a
virtual file for returning results from a GMT module *and* you are
using a :ref:`GMT_MATRIX <struct-matrix>` or :ref:`GMT_VECTOR <struct-vector>`
as your container, you may prescribe
the data type used for the underlying arrays.  The constants below
can be added to the ``direction`` argument in order to change the
default data types [float for matrix and double for vector].

.. _tbl-viatypes:

    +------------------+------------------------------------------+
    | constant         | description                              |
    +==================+==========================================+
    | GMT_VIA_CHAR     | Select GMT_CHAR as array type            |
    +------------------+------------------------------------------+
    | GMT_VIA_UCHAR    | Select GMT_UCHAR as array type           |
    +------------------+------------------------------------------+
    | GMT_VIA_SHORT    | Select GMT_SHORT as array type           |
    +------------------+------------------------------------------+
    | GMT_VIA_USHORT   | Select GMT_USHORT as array type          |
    +------------------+------------------------------------------+
    | GMT_VIA_INT      | Select GMT_INT as array type             |
    +------------------+------------------------------------------+
    | GMT_VIA_UINT     | Select GMT_UINT as array type            |
    +------------------+------------------------------------------+
    | GMT_VIA_LONG     | Select GMT_LONG as array type            |
    +------------------+------------------------------------------+
    | GMT_VIA_ULONG    | Select GMT_ULONG as array type           |
    +------------------+------------------------------------------+
    | GMT_VIA_FLOAT    | Select GMT_FLOAT as array type           |
    +------------------+------------------------------------------+
    | GMT_VIA_DOUBLE   | Select GMT_DOUBLE as array type          |
    +------------------+------------------------------------------+

    Flags to select the type of arrays used in output GMT_MATRIX or GMT_VECTOR.

Footnotes
---------

.. [1]
   or via a very confusing and ever-changing myriad of low-level library
   functions for bold programmers.

.. [2]
   Currently, C/C++, FORTRAN, MATLAB and Julia are being tested.

.. [3]
   This may change in later releases.

.. [4]
   However, there is no thread-support yet, so you will need to manage your
   own threads.

.. ------------------------------------- Examples code -------------------

.. |ex_resource_init| raw:: html

   <a href="#openModal">Example</a>
   <div id="openModal" class="modalDialog">
    <div>
        <a href="#close" title="Close" class="close">X</a>
        <h2>Resource initialization example</h2>
        <p>
        </p>
    </div>
   </div>
