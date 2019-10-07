.. index:: ! mgd77header

***********
mgd77header
***********

.. only:: not man

    mgd77header - Generate MGD77 header from data records

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mgd77header** *NGDC-id.a77*
[ |-H|\ *headervalues.txt* ]
[ |-M|\ **f**\ [*item*]\|\ **r**\ \|\ **t** ]
[ |SYN_OPT-V| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**mgd77header** generates an MGD77 header by reading A77 data (i.e.,
the data record portion of MGD77 files), determines temporal and spatial
extents, ten degree boxes crossed, and data columns present. Optionally,
it can also read an input file consisting of header field values (**-H**\ )
to be included in program output. Header field values determined from
data and read from input are output in either MGD77 format or as a list.

Required Arguments
------------------

.. include:: explain_ncid.rst_

Optional Arguments
------------------

.. _-H:

**-H**\ *headervalues.txt*
    Obtain header field values from the input text file. Each row of
    the input file should consist of a header field name and its
    desired value, separated by a space. See below for a sample header
    file and for the full list of header field names.

.. _-M:

**-Mf**\ [*item*]\|\ **r**\ \|\ **t**
    List the meta-data (header) for
    each cruise. Append **f** for a formatted display. This will list
    individual parameters and their values, one entry per output line,
    in a format that can be searched using standard UNIX text tools.
    Alternatively, append the name of a particular parameter (you only
    need to give enough characters - starting at the beginning - to
    uniquely identify the item). Give - to display the list of all
    parameter names. You may also specify the number of a parameter. For
    the raw, punchcard-formatted MGD77 original header block, append
    **r** instead. For the M77T format, append **t** instead.
 
.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_
    
.. include:: ../../explain_help.rst_

Header Item Sample File
-----------------------

Format_Acronym MGD77

Source_Institution SOEST - UNIV. OF HAWAII

Country USA

Platform_Name KILO MOANA

Platform_Type_Code 1

Platform_Type SHIP

Names of All MGD77 Header Fields
--------------------------------

Survey_Identifier

Format_Acronym

Data_Center_File_Number

Parameters_Surveyed_Code

File_Creation_Year

File_Creation_Month

File_Creation_Day

Source_Institution

Country

Platform_Name

Platform_Type_Code

Platform_Type

Chief_Scientist

Project_Cruise_Leg

Funding

Survey_Departure_Year

Survey_Departure_Month

Survey_Departure_Day

Port_of_Departure

Survey_Arrival_Year

Survey_Arrival_Month

Survey_Arrival_Day

Port_of_Arrival

Navigation_Instrumentation

Geodetic_Datum_Position_Determination_Method

Bathymetry_Instrumentation

Bathymetry_Add_Forms_of_Data

Magnetics_Instrumentation

Magnetics_Add_Forms_of_Data

Gravity_Instrumentation

Gravity_Add_Forms_of_Data

Seismic_Instrumentation

Seismic_Data_Formats

Format_Type

Format_Description

Topmost_Latitude

Bottommost_Latitude

Leftmost_Longitude

Rightmost_Longitude

Bathymetry_Digitizing_Rate

Bathymetry_Sampling_Rate

Bathymetry_Assumed_Sound_Velocity

Bathymetry_Datum_Code

Bathymetry_Interpolation_Scheme

Magnetics_Digitizing_Rate

Magnetics_Sampling_Rate

Magnetics_Sensor_Tow_Distance

Magnetics_Sensor_Depth

Magnetics_Sensor_Separation

Magnetics_Ref_Field_Code

Magnetics_Ref_Field

Magnetics_Method_Applying_Res_Field

Gravity_Digitizing_Rate

Gravity_Sampling_Rate

Gravity_Theoretical_Formula_Code

Gravity_Theoretical_Formula

Gravity_Reference_System_Code

Gravity_Reference_System

Gravity_Corrections_Applied

Gravity_Departure_Base_Station

Gravity_Departure_Base_Station_Name

Gravity_Arrival_Base_Station

Gravity_Arrival_Base_Station_Name

Number_of_Ten_Degree_Identifiers

Ten_Degree_Identifier

Additional_Documentation_1

Additional_Documentation_2

Additional_Documentation_3

Additional_Documentation_4

Additional_Documentation_5

Additional_Documentation_6

Additional_Documentation_7

Examples
--------

To generate an MGD77 header from A77 input, try

   ::

    gmt mgd77header km0201 -Hkmheaderitems.txt -Mf > km0201.h77

See Also
--------

:doc:`mgd77info`,
:doc:`mgd77list`,
:doc:`mgd77manage`,
:doc:`mgd77path`,
:doc:`mgd77track`,
:doc:`x2sys_init <../x2sys/x2sys_init>`

References
----------

The Marine Geophysical Data Exchange Format - MGD77, see
`http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt. <http://www.ngdc.noaa.gov/mgg/dat/geodas/docs/mgd77.txt.>`_
