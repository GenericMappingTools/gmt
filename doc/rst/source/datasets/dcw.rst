The Digital Chart of the World Data (DCW)
=========================================

.. figure:: /_images/dcw-figure.png
   :height: 331 px
   :width: 650 px
   :align: center
   :scale: 100 %

The Digital Chart of the World (DCW) is a comprehensive 1:1,000,000 scale
vector basemap of the world. The charts were designed to meet the needs
of pilots and air crews in medium-and low-altitude en route navigation
and to support military operational planning, intelligence briefings,
and other needs. For basic background information about DCW, see the
`<http://en.wikipedia.org/wiki/Digital_Chart_of_the_World>`_ Wikipedia entry.

DCW-GMT is an enhancement to DCW in a few ways:

#. It contains more state boundaries (the largest 8 countries are now represented).
#. The data have been reformatted to save space and are distributed as a single deflated netCDF-4 file.

DCW-GMT is an optional install.  If you did install it then you can access the DCW data for plotting
or analysis via the :doc:`/coast` module.  You can also use the ISO 2-character codes for countries
as a way to specify map domains via the **-R** option.  For instance, to make a map showing France
with a region rounded to the nearest 2 degrees in longitude and latitude, you can run::

    gmt coast -RFR+r2 -Glightgray -B -pdf france

If we in addition want to paint the landmass of France blue, we run::

    gmt coast -RFR+r2 -Glightgray -B -EFR+gblue -pdf france

To access states without countries you must use the *country.state* syntax.  See the
:doc:`/coast` documentation for details.  For instance, to make a map of the US and
show Texas and Mississippi as red states, try::

    gmt coast -RUS+r2 -Glightgray -B -EUS.TX,US.MS+gred -pdf us

Notes:
------

If you are building GMT from source then you should set the parameter
DCW_ROOT in the cmake/ConfigUser.cmake to point to the directory where
dcw-gmt.nc has been placed.  If you add this file after GMT installation
was completed then you can always have GMT find it by placing it in your
user ~/.gmt directory or by setting the DIR_DCW parameter in the
gmt.conf settings.
    
DCW-GMT is released under the GNU Lesser General Public License.
