.. _OGR_compat:

The GMT Vector Data Format for OGR Compatibility
================================================

Background
----------

The National Institute for Water and Atmospheric Research (NIWA) in New
Zealand has funded the implementation of a GMT driver (read and write)
for the OGR package. OGR is an Open Source toolkit for accessing or
reformatting vector (spatial) data stored in a variety of formats and is
part of the. The intention was to enable the easy rendering (using
GMT) of spatial data held in non-\ GMT formats, and the export of
vector data (e.g., contours) created by GMT for use with other GIS and
mapping packages. While **ogr2ogr** has had the capability to write
this new format since 2009, GMT 4 did not have the capability to use
the extra information.

GMT now allows for more advanced vector data, including donut
polygons (polygons with holes) and aspatial attribute data. At the same
time, the spatial data implementation will not disrupt older GMT 4
programs since all the new information are written via comments.

The identification of spatial feature types in GMT files generally
follows the technical description, (which is largely consistent with the
OGC SFS specification). This specification provides for non-topological
point, line and polygon (area) features, as well as multipoint,
multiline and multipolygon features, and was written by
`Brent Wood <http://www.niwa.co.nz/key-contacts/brent-wood/>`_
based on input from Paul Wessel and others on the GMT team.

The OGR/GMT format
------------------

Several key properties of the OGR/GMT format is summarized below:

-  All new data fields are stored as comment lines, i.e., in lines
   starting with a "#". OGR/GMT files are therefore compatible with
   GMT 4 binaries, which will simply ignore this new information.

-  To be consistent with current practice in GMT, data fields are
   represented as whitespace-separated strings within the comments, each
   identified by the "@" character as a prefix, followed by a single
   character identifying the content of the field. To avoid confusion
   between words and strings, the word (field) separator within strings
   will be the "\|" (pipe or vertical bar) character.

-  Standard UNIX "\\" escaping is used, such as \\n for newline in a string.

-  All new data are stored before the spatial data (coordinates) in the
   file, so when any GMT program is processing the coordinate data
   for a feature, it will already have parsed any non-spatial
   information for each feature, which may impact on how the spatial
   data is treated (e.g., utilizing the aspatial attribute data for a
   feature to control symbology).

-  The first comment line must specify the version of the OGR/GMT data
   format, to allow for future changes or enhancements to be supported
   by future GMT programs. This document describes v1.0.

-  For consistency with other GIS formats (such as shapefiles) the
   OGR/GMT format explicitly contains a field specifying whether the
   features are points, linestrings or polygons, or the "multi" versions
   of these. (Other shapefile feature types will not be supported at
   this stage). At present, GMT programs are informed of this via
   command line parameters. This will now be explicit in the data file,
   but does not preclude command line switches setting symbologies for
   plotting polygons as lines (perimeters) or with fills, as is
   currently the practice.

-  Note that what is currently called a "multiline" (multi-segment) file
   in GMT parlance is generally a set of "lines" in shapefile/OGR
   usage. A multiline in this context is a single feature comprising
   multiple lines. For example, all the transects from a particular
   survey may be stored as lines, each with it's own attribute set, such
   as transect number, date/time, etc. They may also be stored as a
   single multiline feature with one attribute set, such as trip ID.
   This difference is explicitly stored in the data in OGR/shapefiles,
   but currently specified only on the command line in GMT. This
   applies also to points and polygons. The GMT equivalent to
   {multipoint, multiline, multipolygon} datatypes is multiple
   GMT files, each comprising a single {multipoint, multiline,
   multipolygon} feature.

-  The new GMT vector data files includes a header comment specifying
   the type of spatial features it contains, as well as the description
   of the aspatial attribute data to be associated with each feature.
   Unlike the shapefile format, which stores the spatial and aspatial
   attribute data in separate files, the GMT format will store all
   data in a single file.

-  All the features in a GMT file must be of the same type.

OGR/GMT Metadata
----------------

Several pieces of metadata information must be present in the header of
the OGR/GMT file, followed by both spatial and aspatial data. In this
section we look at the metadata.

Format version
~~~~~~~~~~~~~~

The comment header line will include a version identifier providing for
possible different versions in future. It is indicated by the **@V**
sequence.

+-----------+---------------+--------------------------------------------------------------------+
| **Code**  | **Argument**  | **Description**                                                    |
+===========+===============+====================================================================+
| V         | GMT1.0        | Data in this file is stored using v1.0 of the OGR/GMT data format  |
+-----------+---------------+--------------------------------------------------------------------+

An OGR/GMT file must therefore begin with the line

   ::

    # @VGMT1.0

Parsing of the OGR/GMT format is only activated if the version
code-sequence has been found.

Geometry types
~~~~~~~~~~~~~~

The words and characters used to specify the geometry type (preceded by
the **@G** code sequence on the header comment line), are listed in
Table :ref:`geometries <tbl-geometries>`.

.. _tbl-geometries:

+----------+-----------------+---------------------------------------------------------------------------------------+
| **Code** | **Geometry**    | **Description**                                                                       |
+==========+=================+=======================================================================================+
| G        | POINT           | File with point features                                                              |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | (Each point will have it's own attribute/header line preceding the point coordinates) |
+----------+-----------------+---------------------------------------------------------------------------------------+
| G        | MULTIPOINT      | File with a single multipoint feature                                                 |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | (All the point features are a single multipoint, with the same attribute/header       |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | information)                                                                          |
+----------+-----------------+---------------------------------------------------------------------------------------+
| G        | LINESTRING      | File with features comprising multiple single lines                                   |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | (Effectively the current GMT multiline file, each line feature will have it's own     |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | attribute and header data)                                                            |
+----------+-----------------+---------------------------------------------------------------------------------------+
| G        | MULTILINESTRING | File with features comprising a multiline                                             |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | (All the line features in the file are a single multiline feature, only one attribute |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | and header which applies to all the lines)                                            |
+----------+-----------------+---------------------------------------------------------------------------------------+
| G        | POLYGON         | File with one or more polygons                                                        |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | (Similar to a line file, except the features are closed polygons)                     |
+----------+-----------------+---------------------------------------------------------------------------------------+
| G        | MULTIPOLYGON    | File with a single multipolygon                                                       |
+----------+-----------------+---------------------------------------------------------------------------------------+
|          |                 | (Similar to a GMT multiline file, except the feature is a closed multipolygon)        |
+----------+-----------------+---------------------------------------------------------------------------------------+

An example GMT polygon file header using this specification (in format 1.0) is

   ::

    # @VGMT1.0 @GPOLYGON

Domain and map projections
~~~~~~~~~~~~~~~~~~~~~~~~~~

The new format will also support region and projection information. The
region will be stored in GMT **-R** format (i.e., **-R**\ *W/E/S/N*,
where the *W/E/S/N* values represent the extent of features); the **@R**
code sequence marks the domain information. A sample region header is:

   ::

    # @R150/190/-45/-54

Projection information will be represented as four optional strings,
prefixed by **@J** (J being the GMT character for projection
information. The **@J** code will be followed by a character identifying
the format, as shown in Table :ref:`projectspec <tbl-projectspec>`.

.. _tbl-projectspec:

+------------+-------------------------------------------------------------------------------------------------+
| **Code**   | **Projection Specification**                                                                    |
+============+=================================================================================================+
| @Je        | EPSG code for the projection                                                                    |
+------------+-------------------------------------------------------------------------------------------------+
| @Jg        | A string representing the projection parameters as used by GMT                                  |
+------------+-------------------------------------------------------------------------------------------------+
| @Jp        | A string comprising the PROJ parameters representing the projection parameters                  |
+------------+-------------------------------------------------------------------------------------------------+
| @Jw        | A string comprising the OGR WKT (well known text) representation of the projection parameters   |
+------------+-------------------------------------------------------------------------------------------------+

Sample projection strings are:

   ::

    # @Je4326 @JgX @Jp"+proj=longlat +ellps=WGS84+datum=WGS84 +no_defs"
    # @Jw"GEOGCS[\"WGS84\",DATUM[\"WGS_1984\",SPHEROID\"WGS84\",6378137,\
    298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],
    AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,\
    AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,\
    AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]"

Note that an OGR-generated file will not have a **@Jg** string, as OGR
does not have any knowledge of the GMT projection specification
format. GMT supports at least one of the other formats to provide
interoperability with other Open Source related GIS software packages.
One relatively simple approach, (with some limitations), would be a
lookup table matching EPSG codes to GMT strings.

Declaration of aspatial fields
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The string describing the aspatial field names associated with the
features is flagged by the **@N** prefix.

+------------+-----------------------------+-----------------------------------------------------------------+
| **Code**   | **Argument**                | **Description**                                                 |
+============+=============================+=================================================================+
| N          | word\|\ word\|\ word        | A "\|" -separated string of names of the attribute field names  |
+------------+-----------------------------+-----------------------------------------------------------------+

Any name containing a space must be quoted. The **@N** selection must be
combined with a matching string specifying the data type for each of the
named fields, using the **@T** prefix.

+------------+-----------------------------+-------------------------------------------------------------+
| **Code**   | **Argument**                | **Description**                                             |
+============+=============================+=============================================================+
| T          | word\|\ word\|\ word        | A "\|" -separated string of the attribute field data types  |
+------------+-----------------------------+-------------------------------------------------------------+

Available datatypes should largely follow the shapefile (DB3)
specification, including **string**, **integer**, **double**,
**datetime**, and **logical** (boolean). In OGR/GMT vector files, they
will be stored as appropriately formatted text strings.

An example header record containing all these is

   ::

    # @VGMT1.0 @GPOLYGON @Nname|depth|id @Tstring|double|integer

OGR/GMT Data
------------

All generic fields must be at the start of the file before any
feature-specific content (feature attribute data follow the metadata, as
do the feature coordinates, separated by a comment line comprising "#
FEATURE_DATA". Provided each string is formatted as specified, and
occurs on a line prefixed with "#" (i.e., is a comment), the format is
free form, in that as many comment lines as desired may be used, with
one or more parameter strings in any order in any line. E.g., one
parameter per line, or all parameters on one line.

Embedding aspatial data
~~~~~~~~~~~~~~~~~~~~~~~

Following this header line is the data itself, both aspatial and
spatial. For line and polygon (including multiline and multipolygon)
data, features are separated using a predefined character, by default
">". For point (and multipoint) data, no such separator is required. The
comment line containing the aspatial data for each feature will
immediately precede the coordinate line(s). Thus in the case of lines
and polygons, it will immediately follow the ">" line. The data line
will be a comment flag ("#") followed by **@D**, followed by a string of
"\|"-separated words comprising the data fields defined in the
header record.

To allow for names and values containing spaces, such string items among
the **@N** or **@D** specifiers must be enclosed in double quotes.
(Where double quotes or pipe characters are included in the string, they
must be escaped using "\\"). Where any data values are
null, they will be represented as no characters between the field
separator, (e.g., #@D\|\|\|). A Sample header
and corresponding data line for points are

   ::

    # @VGMT1.0 @GPOINT @Nname|depth|id @Tstring|double|integer
    # @D"Point 1"|-34.5|1

while for a polygon it may look like

   ::

    # @VGMT1.0 @GPOLYGON @Nname|depth|id @Tstring|double|integer
    >
    # @D"Area 1"|-34.5|1

Polygon topologies
~~~~~~~~~~~~~~~~~~

New to GMT is the concept of polygon holes. Most other formats do
support this structure, so that a polygon is specified as a sequence of
point defining the perimeter, optionally followed by similar coordinate
sequences defining any holes (the "donut" polygon concept).

To implement this in a way which is compatible with previous
GMT versions, each polygon feature must be able to be identified as
the outer perimeter, or an inner ring (hole). This is done using a
**@P** or **@H** on the data comment preceding the polygon coordinates.
The **@P** specifies a new feature boundary (perimeter), any following
**@H** polygons are holes, and must be within the preceding **@P**
polygon (as described in the shapefile specification). Any **@H**
polygons will NOT have any **@D** values, as the aspatial attribute data
pertain to the entire feature, the **@H** polygons are not new polygons,
but are merely a continuation of the definition of the same feature.
**Note**: The perimeter and the hole(s) must have different handedness.
E.g., if the perimeter goes counter-clockwise then the holes must go
clockwise, and vice versa.  This is important to follow if you are creating
such features manually.

Examples
--------

Sample point, line and polygon files are (the new data structures are in
lines starting with "#" in strings prefixed with "@"). Here is a typical
point file:

   ::

    # @VGMT1.0 @GPOINT @Nname|depth|id
    # @Tstring|double|integer
    # @R178.43/178.5/-57.98/-34.5
    # @Je4326
    # @Jp"+proj=longlat +ellps=WGS84 +datum=WGS84+no_defs"
    # FEATURE_DATA
    # @D"point 1"|-34.5|1
    178.5 -45.7
    # @D"Point 2"|-57.98|2
    178.43 -46.8
    ...

Next is an example of a line file:

   ::

    # @VGMT1.0 @GLINESTRING @Nname|depth|id
    # @Tstring|double|integer
    # @R178.1/178.6/-48.7/-45.6
    # @Jp"+proj=longlat +ellps=WGS84 +datum=WGS84+no_defs"
    # FEATURE_DATA
    > -W0.25p
    # @D"Line 1"|-50|1
    178.5 -45.7
    178.6 -48.2
    178.4 -48.7
    178.1 -45.6
    > -W0.25p
    # @D"Line 2"|-57.98|$
    178.43 -46.8
    ...

Finally we show an example of a polygon file:

   ::

    # @VGMT1.0 @GPOLYGON @Npolygonname|substrate|id @Tstring|string|integer
    # @R178.1/178.6/-48.7/-45.6
    # @Jj@Jp"+proj=longlat +ellps=WGS84 +datum=WGS84+no_defs"
    # FEATURE_DATA
    > -Gblue -W0.25p
    # @P
    # @D"Area 1"|finesand|1
    178.1 -45.6
    178.1 -48.2
    178.5 -48.2
    178.5 -45.6
    178.1 -45.6
    >
    # @H
    # First hole in the preceding perimeter, so is technically still
    # part of the same geometry, despite the preceding > character.
    # No attribute data is provided, as this is inherited.
    178.2 -45.4
    178.2 -46.5
    178.4 -46.5
    178.4 -45.4
    178.2 -45.4
    >
    # @P
    ...
