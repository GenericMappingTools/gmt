# GMT Code Conventions

## Contents

* [The documented external GMT API](#the-api)
* [The undocumented internal GMT library](#the-developer-library)

The GMT source code contains a large variety of functions, variables,
structures, macros, and constants (including enums).  Only a small
part of these are exposed via our external API, while other items are used
in the operations of modules or other internal functions via the internal
library.  To keep these as organized as possible, we need follow our naming
guidelines.  As of this writing, we are violating these guidelines in places
and will seek to reach a better level of compliance as we go forward.

## The API

The official API provides declaration for all items accessible by
the users of the API (e.g., the external interfaces to GMT from
MATLAB, Python, and Julia). The are all accessed by including
[gmt.h](src/gmt.h) which includes [gmt_resources.h](src/gmt_resources.h).

### GMT API Functions

All API functions (about 84) use the prefix GMT_ and are listed in [gmt.h](src/gmt.h).
As an example, the function to read data is called **GMT_Read_Data**.


### GMT API Data Structures

External tools written in C or C++ may wish to handle the GMT core nine
data structures for holding grids, images, table data, color palettes,
and more in memory.  These structures are named in upper-case starting
with **GMT_** as well.  For example, the structure that holds a GMT grid
and its meta data is called **struct GMT_GRID**. The complete list of structures that
are defined are **GMT_GRID** (with **GMT_GRID_HEADER**), **GMT_DATASET** (with **GMT_DATATABLE** and
**GMT_DATASEGMENT** in a hierarchy), **GMT_PALETTE** (with **GMT_LUT** and **GMT_BFN)**,
**GMT_IMAGE** (also with **GMT_GRID_HEADER**), **GMT_POSTSCRIPT**, **GMT_VECTOR**, **GMT_MATRIX**,
**GMT_OPTION**, and **GMT_RESOURCE**.

### GMT API Enums

There is a large number (currently 232) of *enums* in the GMT API and external
developers can access the value of all of them via **GMT_Get_Enum**; developers
in C and C++ can also use them directly since they are exported by [gmt_resources.h](src/gmt_resources.h).
These are all written in upper-case and start with **GMT_** as well.

### GMT API types

There is only one data type defined in the API and that is *gmt_grdfloat*.  This
is typeset to *float* but developers with special needs can change this to *double*.
This means all GMT grids would use double precision [default is float].

## The Developer Library

Developers writing in C or C++ may choose to include [gmt_dev.h](src/gmt_dev.h)
in their codes. This opens up access to a myriad of additional, lower-level functions,
macros, structures, and enums and constants.  These are not documented and
probably will never be documented.  Unlike for the API, there are no guarantees given that
these functions and features will remain unchanged or even be present in future release.
However, we do have naming conventions for these as well since there are different levels
of access and exposure.

### Use by modules

Functions needed by one or more modules are named with a leading lower-case
**gmt_**.  As an example, the function **gmt_getpen** parses a string argument to an
option that expects a valid pen syntax.  All of these functions are declared
in [gmt_prototypes.h](src/gmt_prototypes.h)
and prefaced by **EXTERN_MSC** so they are properly exported
under Windows.  Because these are exported, external developers that include
[gmt_dev.h](src/gmt_dev.h) have access to all these functions.

### Internal library use

The GMT API is based on > 40 separate C files loosely organized by topic.
E.g., [gmt_proj.c](src/gmt_proj.c) has all the low-level map projection, all plot functions are kept in
[gmt_plot.c](src/gmt_plot.c), and so on. Many of the functions in these files need
access to functions in the other files and thus most also be exported properly.
These functions are all given names starting with **gmtlib_** and are listed
in [gmt_internals.h](src/gmt_internals.h).

### Single file use

Helper functions that are only used within a single C file should not be
exported and should all be labeled **static** (i.e., with **GMT_LOCAL**).  Furthermore,
the names of these should derive from the file they are in.  For example,
a low-level function in gmt_support.c that deals with parsing any fill
argument containing a pattern is called **gmtsupport_parse_pattern**.  This
function cannot be called outside gmt_support.c.  This naming convention makes
it easy to find functions when debugging is needed.  The only exception to this
convention is the four static functions present in all modules: New_Ctrl, Free_Ctrl,
usage, and parse.

### GMT C macros

GMT defines numerous C macros for things were a functions would be overkill, given
the extra overhead in calling a function.  Again, access to all GMT macros is given by
including [gmt_dev.h](src/gmt_dev.h).
All GMT macros have the leading prefix **gmt_M_**, where the **M** indicates it is a macro
and not a function.

### GMT inline functions

Some lower-level include file declares static inline functions that are only accessible
by the functions in the file doing the including. These are used for tasks where macros
may be too cumbersome.

### API structs, enums, and constants

The lower-level functions use lower-level utility structures that are not
part of the API but they are exposed to developers via [gmt_dev.h](src/gmt_dev.h).
As just one example, *struct* **GMT_PEN** is the internal structure that holds all the
settings that describe a pen used to draw lines in GMT.  These are also
named in upper-case starting with **GMT_**.  There are also many enums available
and there are also upper-case and start with **GMT_**.  Finally, many constants
(e.g., *#define GMT_TEXT_CLEARANCE 15*) are set via [gmt_dev.h](src/gmt_dev.h)
and are thus available to C/C++ developers.  Unlike the API enums, there is no method
to access these values via a GMT_Get_Enum-type function.

### Private structs, enums, and constants

Structures, enums and constants just used in one modules should have names that
start with the uppercase name of the module, e.g., *struct GRDEDIT_A A* is the
sub-structure that handles the -A option in grdedit.

