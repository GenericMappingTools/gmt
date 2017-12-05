GMT geological symbols
======================

What's this?
------------

This is a collection of geological symbols for the Generic Mapping Tools (GMT) software.
The symbols comprises the most widely symbols used in geological mapping for bedding,
cleavage, foliation, joints and lineations.
The collection currently comprises 25 symbols, being most of them dynamic, using parameters
given as variables in columns of the input data. The details of the parameters used for
each symbol are given below.
Author of the symbols: José A. Álvarez-Gómez (jaag@ucm.es). I greatly appreciate your
suggestions and feedback.
These symbols are freely available under a GNU Library General Public License (version 3, or later).

 
How to use the symbols?
-----------------------

The symbols represent basically the orientation of a plane or a line in the space. For the
plane the parameters used are the strike in Right Hand Rule (RHR) convention and the dip.
The strike can be defined as the trend of a horizontal line on an inclined plane. It is
marked by the line of intersection with a horizontal plane. The RHR establishes that the
measured angle is clockwise from the north (azimuth) of the line of intersection between
the plane and the horizontal, with the plane dipping orthogonally to the right when looking
in the direction of the strike azimuth. The dip is the inclination of the steepest line on
a plane and is perpendicular to the strike direction. For the lines the trend and plunge
are used. The trend is measured clockwise from the north and the plunge is the angle of
the line with the horizontal.

In order to use the symbols in GMT you need to use the program :doc:`plot </plot>`, using the custom
symbol type **-Sk**\ [*symbolname*\ ]/\ *size*\ ; where symbolname is one of the short
names of the geological symbols shown in the table below. In addition to the location of the
symbol (x,y) given on the first two columns of the input file, you will need additional
parameters for some of the symbols. You can also use as variables the size and color of
the symbol as in any symbol plotted with :doc:`plot </plot>`. The table lists the parameters needed for
each symbol; see the gallery for visual representation.

.. _tbl-Geology:

    +---------------------------------------------------------------+--------------------+-------------------+
    | Symbol Description                                            | Symbol Name        | Parameters needed |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of beds                         | geo-plane          | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Horizontal beds                                               | geo-plane_hor      |                   |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike of vertical beds                                       | geo-plane_vert     | Strike            |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of overturned beds              | geo-plane_inv      | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of bed with rake of lineation   | geo-plane_rake     | Strike, Dip, Rake |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike and dip direction of gently dipping beds               | geo-plane_gentle   | Strike            |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike and dip direction of moderately dipping beds           | geo-plane_medium   | Strike            |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike and dip direction of steeply dipping beds              | geo-plane_steep    | Strike            |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of crenulated or undulated beds | geo-plane_und      | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of foliation                    | geo-foliation      | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Horizontal foliation                                          | geo-foliation_hor  |                   |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike of vertical foliation                                  | geo-foliation_vert | Strike            |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of cleavage                     | geo-cleavage       | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Horizontal cleavage                                           | geo-cleavage_hor   |                   |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike of vertical cleavage                                   | geo-cleavage_vert  | Strike            |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of foliation 2                  | geo-foliation-2    | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of foliation 3                  | geo-foliation-3    | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike, dip direction and dip of joints                       | geo-joint          | Strike, Dip       |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Horizontal joints                                             | geo-joint_hor      |                   |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Strike of vertical joints                                     | geo-joint_vert     | Strike            |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Trend and plunge of lineation                                 | geo-lineation      | Trend, Plunge     |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Vertical lineation                                            | geo-lineation_vert |                   |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Horizontal lineation                                          | geo-lineation_hor  | Trend             |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Trend and plunge of lineation 2                               | geo-lineation-2    | Trend, Plunge     |
    +---------------------------------------------------------------+--------------------+-------------------+
    | Trend and plunge of lineation 3                               | geo-lineation-3    | Trend, Plunge     |
    +---------------------------------------------------------------+--------------------+-------------------+

    Symbols of the GMT geological symbols collection. The symbol short name is the name that should 
    be used in :doc:`plot </plot>` **-Sk**\ *symbolname*\ /*size*. The extra parameters needed for each symbol are
    shown on the order that is expected for the symbol definition.
