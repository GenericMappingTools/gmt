******
grdfft
******

grdfft - Do mathematical operations on grids in the wavenumber (or
frequency) domain

`Synopsis <#toc1>`_
-------------------

**grdfft** *ingrid* **-G**\ *outgrid* [ **-A**\ *azimuth* ] [
**-C**\ *zlevel* ] [ **-D**\ [*scale*\ **\|g**] ] [
**-E**\ [**x\|y**\ ][**w**\ ] ] [ **-F**\ [**x**\ \|\ **y**]\ *params* ]
[ **-I**\ [*scale*\ **\|g**] ] [ **-L** ] [ **-N**\ *stuff* ] [
**-S**\ *scale* ] [ **-T**\ *te/rl/rm/rw/ri* ] [ **-V**\ [*level*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdfft** will take the 2-D forward Fast Fourier Transform and perform
one or more mathematical operations in the frequency domain before
transforming back to the space domain. An option is provided to scale
the data before writing the new values to an output file. The horizontal
dimensions of the grid are assumed to be in meters. Geographical grids
may be used by specifying the **-f** option that scales degrees to
meters. If you have grids with dimensions in km, you could change this
to meters using **grdedit** or scale the output with **grdmath**.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*ingrid*
    2-D binary grid file to be operated on. (See GRID FILE FORMATS
    below).
**-G**\ *outgrid*
    Specify the name of the output grid file or the 1-D spectrum file
    (see **-E**). (See GRID FILE FORMATS below).

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ *azimuth*
    Take the directional derivative in the *azimuth* direction measured
    in degrees CW from north.
**-C**\ *zlevel*
    Upward (for *zlevel* > 0) or downward (for *zlevel* < 0) continue
    the field *zlevel* meters.
**-D**\ [*scale*\ **\|g**]
    Differentiate the field, i.e., take d(field)/dz. This is equivalent
    to multiplying by kr in the frequency domain (kr is radial wave
    number). Append a scale to multiply by (kr \* *scale*) instead.
    Alternatively, append **g** to indicate that your data are geoid
    heights in meters and output should be gravity anomalies in mGal.
    [Default is no scale].
**-E**\ [**x\|y**\ ][**w**\ ]
    Estimate power spectrum in the radial direction. Place **x** or
    **y** immediately after **-E** to compute the spectrum in the x or y
    direction instead. No grid file is created; f (i.e., frequency or
    wave number), power[f], and 1 standard deviation in power[f] are
    written to file set by **-G** [stdout]. Append **w** to write
    wavelength instead of frequency.
**-F**\ [**x**\ \|\ **y**]\ *params*
    Filter the data. Place **x** or **y** immediately after **-F** to
    filter *x* or *y* direction only; default is isotropic. Choose
    between a cosine-tapered band-pass, a Gaussian band-pass filter, or
    a Butterworth band-pass filter. Cosine-taper: Specify four
    wavelengths *lc*/*lp*/*hp*/*hc* in correct units (see **-fg**) to
    design a bandpass filter: wavelengths greater than *lc* or less than
    *hc* will be cut, wavelengths greater than *lp* and less than *hp*
    will be passed, and wavelengths in between will be cosine-tapered.
    E.g., **-F**\ 1000000/250000/50000/10000 **-fg** will bandpass,
    cutting wavelengths > 1000 km and < 10 km, passing wavelengths
    between 250 km and 50 km. To make a highpass or lowpass filter, give
    hyphens (-) for *hp*/*hc* or *lc*/*lp*. E.g., **-Fx**-/-/50/10 will
    lowpass *x*, passing wavelengths > 50 and rejecting wavelengths <
    10. **-Fy**\ 1000/250/-/- will highpass *y*, passing wavelengths <
    250 and rejecting wavelengths > 1000. Gaussian band-pass: Append
    *lo*/*hi*, the two wavelengths in correct units (see **-fg**) to
    design a bandpass filter. At the given wavelengths the Gaussian
    filter weights will be 0.5. To make a highpass or lowpass filter,
    give a hyphen (-) for the *hi* or *lo* wavelength, respectively.
    E.g., **-F**-/30 will lowpass the data using a Gaussian filter with
    half-weight at 30, while **-F**\ 400/- will highpass the data.
    Butterworth band-pass: Append *lo*/*hi*/*order*, the two wavelengths
    in correct units (see **-fg**) and the filter order (an integer) to
    design a bandpass filter. At the given wavelengths the Butterworth
    filter weights will be 0.5. To make a highpass or lowpass filter,
    give a hyphen (-) for the *hi* or *lo* wavelength, respectively.
    E.g., **-F**-/30/2 will lowpass the data using a 2nd-order
    Butterworth filter, with half-weight at 30, while **-F**\ 400/-/2
    will highpass the data.
**-I**\ [*scale*\ **\|g**]
    Integrate the field, i.e., compute integral\_over\_z (field \* dz).
    This is equivalent to divide by kr in the frequency domain (kr is
    radial wave number). Append a scale to divide by (kr \* *scale*)
    instead. Alternatively, append **g** to indicate that your data set
    is gravity anomalies in mGal and output should be geoid heights in
    meters. [Default is no scale].
**-L**
    Leave trend alone. By default, a linear trend will be removed prior
    to the transform.
**-N**\ *stuff*
    Choose or inquire about suitable grid dimensions for FFT. **-Nf**
    will force the FFT to use the dimensions of the data. **-Nq** will
    inQuire about more suitable dimensions. **-N**\ *nx/ny* will do FFT
    on array size *nx/ny* (Must be >= grid file size). Default chooses
    dimensions >= data which optimize speed, accuracy of FFT. If FFT
    dimensions > grid file dimensions, data are extended and tapered to
    zero.
**-S**\ *scale*
    Multiply each element by *scale* in the space domain (after the
    frequency domain operations). [Default is 1.0].
**-T**\ *te/rl/rm/rw/ri*
    Compute the isostatic compensation from the topography load (input
    grid file) on an elastic plate of thickness *te*. Also append
    densities for load, mantle, water, and infill in SI units. If *te*
    == 0 then the Airy response is returned. **-T** implicitly sets
    **-L**.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Grid File Formats <#toc6>`_
----------------------------

By default **GMT** writes out grid as single precision floats in a
COARDS-complaint netCDF file format. However, **GMT** is able to produce
grid files in many other commonly used grid file formats and also
facilitates so called "packing" of grids, writing out floating point
data as 1- or 2-byte integers. To specify the precision, scale and
offset, the user should add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. When reading grids, the format is generally automatically
recognized. If not, the same suffix can be added to input grid file
names. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. The **?**\ *varname* suffix can also be used for output
grids to specify a variable name different from the default: "z". See
`**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20 of the GMT
Technical Reference and Cookbook for more information, particularly on
how to read splices of 3-, 4-, or 5-dimensional grids.

`Considerations <#toc7>`_
-------------------------

netCDF COARDS grids will automatically be recognized as geographic. For
other grids geographical grids were you want to convert degrees into
meters, select **-fg**. If the data are close to either pole, you should
consider projecting the grid file onto a rectangular coordinate system
using **grdproject**.

`Examples <#toc8>`_
-------------------

To upward continue the sea-level magnetic anomalies in the file
mag\_0.nc to a level 800 m above sealevel:

grdfft mag\_0.nc -C800 -V -Gmag\_800.nc

To transform geoid heights in m (geoid.nc) on a geographical grid to
free-air gravity anomalies in mGal:

grdfft geoid.nc -Dg -V -Ggrav.nc

To transform gravity anomalies in mGal (faa.nc) to deflections of the
vertical (in micro-radians) in the 038 direction, we must first
integrate gravity to get geoid, then take the directional derivative,
and finally scale radians to micro-radians:

grdfft faa.nc -Ig38 -S1e6 -V -Gdefl\_38.nc

Second vertical derivatives of gravity anomalies are related to the
curvature of the field. We can compute these as mGal/m^2 by
differentiating twice:

grdfft gravity.nc -D -D -V -Ggrav\_2nd\_derivative.nc

The first order gravity anomaly (in mGal) due to the compensating
surface caused by the topography load topo.nc (in m) on a 20 km thick
elastic plate, assumed to be 4 km beneath the observation level can be
computed as

grdfft topo.nc -T20000/2800/3330/1030/2300 -S0.022 -C4000 -Gcomp\_faa.nc

where 0.022 is the scale needed for the first term in Parker’s expansion
for computing gravity from topography (= 2 \* PI \* G \* (rhom - rhol)).

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdedit*\ (1) <grdedit.html>`_ ,
`*grdmath*\ (1) <grdmath.html>`_ ,
`*grdproject*\ (1) <grdproject.html>`_
