Introduction
============

Historical overview
-------------------

Most scientists are familiar with the sequence: *raw data* →
*processing* → *final illustration*.
In order to finalize papers for submission to scientific journals,
prepare proposals, and create illustrations for various
presentations, many scientists spend large amounts of time and money to
create high-quality figures. This process can be tedious and is often
done manually, since available commercial or in-house software usually
can do only part of the job. To expedite this process we introduce the
Generic Mapping Tools (GMT for short), which is a free [2]_ software
package that can be used to manipulate columns of tabular data,
time-series, and gridded data sets, and display these data in a variety
of forms ranging from simple *x*–*y* plots to maps and color-coded,
perspective, and shaded-relief illustrations. GMT uses the
PostScript page description language [*Adobe Systems Inc.*, 1990].
With PostScript, multiple plot files can easily be superimposed to
create arbitrarily complex images in gray tones or full color.
Line drawings, bitmapped images, and text can be easily combined in one
illustration. PostScript plot files are device-independent: The same
file can be printed at 300 dots per inch (dpi) on a cheap
printer or converted to a high-resolution PNG image for online usage.
GMT software is written as a set of command-line tools [3]_ and is
totally self-contained and fully documented. The system is offered free
of charge and is distributed over the Internet
[*Wessel and Smith, 1991; 1995; 1998*; *Wessel et al., 2013*; *Wessel et al., 2019*].
The PostScript plots are easily converted to other formats, such as PDF
or any raster image [4]_.

The original version 1.0 of GMT was released in the summer of 1988
when the authors were graduate students at Lamont-Doherty Earth
Observatory of Columbia University. During our tenure as graduate
students, LDEO changed its computing environment to a distributed
network of UNIX workstations, and we wrote GMT to run in this
environment. It became a success at LDEO, and soon spread to numerous
other institutions in the US, Canada, Europe, and Japan. The current
version benefits from the many suggestions contributed by users of the
earlier versions, and now includes more than 100 tools, more than 30
projections, and many other new, more flexible features. GMT provides
scientists with a variety of tools for data manipulation and display,
including routines to sample, filter, compute spectral estimates, and
determine trends in time series, grid or triangulate arbitrarily spaced
data, perform mathematical operations (including filtering) on 2-D data
sets both in the space and frequency domain, sample surfaces along
arbitrary tracks or onto a new grid, calculate volumes, and find trend
surfaces. The plotting programs will let the user make linear,
log\ :math:`_{10}`, and :math:`x^a - y^b` diagrams, polar
and rectangular histograms, maps with filled continents and coastlines
choosing from many common map projections, contour plots, mesh plots,
monochrome or color images, and artificially illuminated shaded-relief
and 3-D perspective illustrations.

GMT is written in the highly portable ANSI C programming language
[*Kernighan and Ritchie*, 1988], is fully POSIX compliant [*Lewine*,
1991], and may be used with any hardware
running some flavor of UNIX. In
writing GMT, we have followed the modular design philosophy of UNIX:
The *raw data* → *processing* → *final illustration* flow is broken
down to a series of elementary steps; each
step is accomplished by a separate GMT or UNIX tool. This modular
approach brings several benefits: (1) only a few programs are needed,
(2) each program is small and easy to update and maintain, (3) each step
is independent of the previous step and the data type and can therefore
be used in a variety of applications, and (4) the programs can be
chained together in shell scripts or with pipes, thereby creating a
process tailored to do a user-specific task. The decoupling of the data
retrieval step from the subsequent massage and plotting is particularly
important, since each institution will typically have its own data base
formats. To use GMT with custom data bases, one has only to write a
data extraction tool which will put out data in a form readable by
GMT (discussed below). After writing the extractor, all other
GMT modules will work as they are.

GMT is thoroughly documented and comes with a technical reference and
cookbook which explains the purpose of the package and its many
features, and provides numerous examples to help new users quickly
become familiar with the operation and philosophy of the system. The
cookbook contains the shell scripts that were used for each example. The online 
GMT Documentation is also home to the extensive technical reference for all programs. 
The programs also have individual manual pages which can be installed as part of the
on-line documentation under the UNIX **man** utility. 
In addition, the programs offer friendly help messages which make
them essentially self-teaching – if a user enters invalid or ambiguous
command arguments, the program will print a warning to the screen with a
synopsis of the valid arguments. All the documentation is available for
web browsing and may be installed at the user's site.

The processing and display routines within GMT are completely general
and will handle any (*x,y*) or (*x,y,z*) data as input. For many
purposes the (*x,y*) coordinates will be (longitude, latitude) but in
most cases they could equally well be any other variables (e.g.,
wavelength, power spectral density). Since the GMT plot tools will
map these (*x,y*) coordinates to positions on a plot or map using a
variety of transformations (linear, log-log, and several map
projections), they can be used with any data that are given by two or
three coordinates. In order to simplify and standardize input and
output, by default GMT uses two file formats only. Arbitrary sequences of (*x,y*)
or (*x,y,z*) data are read from multi-column ASCII tables, i.e., each
file consists of several records, in which each coordinate is confined
to a separate column [5]_. This format is straightforward and allows the
user to perform almost any simple (or complicated) reformatting or
processing task using GMT processing tools (and in a pinch standard UNIX utilities such as **cut**,
**paste**, **grep**, **sed** and **awk**). Two-dimensional data
that have been sampled on an equidistant grid are read and written by
GMT in a binary grid file using the functions provided with the netCDF
library (a free, public-domain software library available separately
from UCAR, the University Corporation of Atmospheric Research [*Treinish
and Gough*, 1987]). This XDR (External Data Representation) based format
is architecture independent, which allows the user to transfer the
binary data files from one computer system to another [6]_.
GMT contains programs that will read ASCII (*x,y,z*) files and produce
grid files. One such program, :doc:`/surface`,
includes new modifications to the gridding algorithm developed by *Smith
and Wessel* [1990] using continuous splines in tension. Optionally, GMT
can also read various binary and netCDF tables, as well as a variety of
grid formats, especially if built with GDAL support.

Most of the programs will produce some form of output, which falls into
four categories. Several of the programs may produce more than one of
these types of output:

*  1-D ASCII Tables — For example, a (*x,y*) series may be
   filtered and the filtered values output. ASCII output is written to
   the standard output stream.

*  2-D binary (netCDF or user-defined) grid files – Programs that grid
   ASCII (*x,y,z*) data or operate on existing grid files produce
   this type of output.

*  PostScript – The plotting programs all use the PostScript page
   description language to define plots. These commands are stored as
   ASCII text and can be edited should you want to customize the plot
   beyond the options available in the programs themselves.

*  Reports – Several GMT programs read input files and report
   statistics and other information. Nearly all programs have an
   optional "verbose" operation, which reports on the progress of
   computation. All programs feature usage messages, which prompt the
   user if incorrect commands have been given. Such text is written to
   the standard error stream and can therefore be separated from ASCII
   table output.

GMT is available over the Internet at no charge. To obtain a copy,
go to the `GMT home page <https://www.generic-mapping-tools.org>`_ and follow instructions.
We also maintain user forums and a bug and feature tracking system on
the same page.

GMT has served a multitude of scientists very well, and their
responses have prompted us to develop these programs even further. It is
our hope that the new version will satisfy these users and attract new
users as well. We present this system to the community in order to
promote sharing of research software among investigators in the US and abroad.

References
----------

*  Adobe Systems Inc., *PostScript Language Reference Manual*, 2nd
   edition, 764, Addison-Wesley, Reading, Massachusetts, 1990.

*  Kernighan, B. W., and D. M. Ritchie, *The C programming language*,
   2nd edition, 272, Prentice-Hall, Englewood Cliffs, New Jersey, 1988.

*  Lewine, D., POSIX programmer's guide, 1st edition, 607, O'Reilly &
   Associates, Sebastopol, California, 1991.

*  Treinish, L. A., and M. L. Gough, A software package for the
   data-independent management of multidimensional data, *EOS Trans.
   AGU*, 68(28), 633–635, 1987. `doi:10.1029/EO068i028p00633 <http://dx.doi.org/10.1029/EO068i028p00633>`_.


Modern and Classic Mode
-----------------------

For almost three decades, GMT scripts have looked remarkably similar.  The options flags
and the general workflow of adding overlays to existing PostScript files have
remained unchanged, and thousands of GMT scripts written in c-shell, bash shell, DOS batch,
and other environments exist and their maintainers expect them to run in the future.
This requirement of backwards compatibility has to some extent stifled our drive to
make GMT easier and safer to use.  Having run dozens of classes introducing GMT to students
and staff, and helped hundreds of practitioners via email or forums over the years, we
have a pretty clear idea of what is difficult.

Given its almost limitless capabilities, GMT has always had a fairly steep learning curve.
The hardest aspects that have percolated to the top of the "rookie error" list include

#. The GMT "cake-baking": Handling the use of **-O**, **-K**, and **-P** to manage PostScript overlays.
#. The PostScript redirection: Creating a new file versus appending to an existing file.
#. Reusing the current region (**-R**) and projection (**-J**) in multi-step scripts by repeating **-R -J** everywhere.
#. Converting the PostScript plot to more desirable graphic formats, such as PDF.

While pondering these facts, we have also started to gain experience with the MATLAB and Octave
toolboxes and the preliminary design of the Python package. We were noticing that
the resulting scripts looked too much like the GMT shell command-line versions, setting
users up for a continuation of the same rookie errors.
The solution to this conundrum was to introduce different *run* modes:
Starting with GMT 6 we introduce a new operating *mode* for GMT named *modern*.  In contrast
to the *classic* (and only) mode available in earlier versions 1-5, the *modern* mode
was designed to eliminate some of the hardest aspects of learning and using GMT.
Depending on how GMT is started it will either be running in *classic* or *modern* mode.
Classic mode is the GMT scripting in use for decades and it will remain the default mode for
command-line work. The *modern* mode invokes simpler rules that eliminate the possibility
of the listed rookie errors and simplifies scripting considerably across all interfaces.
It also imposes a structure and hence not every single classic script can be represented in
modern mode.  Consequently, modern mode is less flexible but much easier to use, and we expect
it will serve the needs of almost all GMT users.  We strongly encourage new users to use the
modern mode.

To defeat the rookie errors listed above, here are the features of *modern* mode:

#. The **-O**, **-K**, and **P** options have been removed.
#. Modules no longer write PostScript to standard output that the users must manage.
   Instead, they write to hidden temporary files.  Checking the status of these files
   is what allows GMT to know if PostScript should be appended or if we are starting
   a new plot.
#. The *modern* mode runs the entire workflow in a unique temporary directory, hence
   numerous scripts can execute simultaneously without interfering, and we can use
   the gmt.history information to automatically supply missing regions (**-R**) and
   projection (**-J**) arguments.
#. When the workflow ends, the hidden PostScript files are automatically completed
   and converted to the chosen graphics format [Default is PDF for command-line work].
#. Page size is now automatically set regardless of size and properly cropped.

Not only does the new rules remove the greatest obstacles to GMT learning, it greatly
simplifies scripting by eliminating needless repetition of options and output filenames.  The
modern mode is activated and deactivated by the new commands **gmt begin** and **gmt end**,
respectively.  Since these are not part of the classic repertoire one cannot
accidentally execute a classic mode script in modern mode (or vice versa).
We will discuss these two commands later.  Finally, there are some new features in GMT that
are only accessible under modern mode, such as subplots, new ways to specify the map domain,
map insets, perform automatic legend creation and placement, create simpler animations, and to
get multiple output formats from the same plot.

The modern mode relies on know what session is being run. If your script is explicitly or
inadvertently creating sub-shells under UNIX then the script could fail.  If this is the
case then you will need to add
export GMT_SESSION_NAME=<some unique string>
before gmt begin starts the script.  This is most easily done by using the **gmt --new-script**
option to print a shell template to the standard output.

Footnotes
---------

.. [2]
   See GNU Lesser General Public License (`<http://www.gnu.org/copyleft/gpl.html>`_)
   for terms on redistribution and modifications.

.. [3]
   The tools can be installed on a variety of platforms - UNIX and non-UNIX alike (see Chapter :doc:`non-unix-platforms`).

.. [4]
   One public-domain RIP is ghostscript, available from `<https://www.ghostscript.com/>`_.

.. [5]
   Programs now also allow for fast, binary multicolumn file i/o.

.. [6]
   While the netCDF format is the default, many other formats are also possible.
