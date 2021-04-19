GMT Modern Mode One-line Commands
=================================

Background
----------

Modern mode simplifies GMT by placing all commands between gmt :doc:`/begin` and gmt :doc:`/end`.
However, for very simple plots, such as a simple coastline map that only requires a call to
a single module, having to place that command between **begin** and **end** makes the documentation
longer and more tedious to maintain.  Because of this, we have implemented a special and quick way
to begin and end GMT modern mode within a single call to a plotting module. When the plot finishes
we automatically call gmt :doc:`/end` with the show command to display the plot(s).

One-liner Syntax
----------------

The syntax for this special mechanism involves a few simple rules:

#. The command must be a plotting command and modern mode syntax (e.g., :doc:`/coast` instead
   of :doc:`/pscoast`, no **-O**, **-K**, **-P** options, etc.) is required.
#. The plot file and type must be specified with the special option **-format** *prefix*,
   where **format** is one or more comma-separated graphics extensions from the list of
   allowable graphics formats :ref:`formats <tbl-formats>`, and *prefix* is the prefix of
   the illustration (the extension is automatically set by the chosen format). Note the
   leading hyphen for the format option.

Examples
--------

To make a Mercator coastline plot of France, painting land blue, adding map frame and
requesting both PDF and png graphics output with prefix France1, try

   ::

    gmt coast -RFR -JM6i -Gblue -B -pdf,png France1

To make a DEM map of France using the default DEM color table and data from the 1m global
grid, making a JPG plot called France2, try

   ::

    gmt grdimage @earth_relief_01m -RFR -JM6i -B -jpg France2

The automatic display of the plot can be deactivated by setting an environmental parameter
named GMT_END_SHOW to off.
