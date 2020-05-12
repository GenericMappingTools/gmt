.. index:: ! develop

**********************************
Developing Modules and Supplements
**********************************

GMT developers do from time to time need to add new modules to the core library,
and occasionally they add an entire new supplement to the official distribution.  In
addition, there are unofficial supplements that are not part of the GMT code base
but needs to be built with GMT (or piggyback off the GMT build process), and then
there are supplements that need to be built by themselves that have dependencies
of various kinds on GMT.  This document discusses the mechanics around those issues.

Adding a new module
-------------------

To add a new module, the starting point is to duplicate the most similar module you can think
of.  It is not essential to get this right, but if your new module deals with grids then
start with one of the grd modules such as :doc:`grd2xyz`, and if it needs to deal with tables
then maybe something like :doc:`gmtselect`.  If plotting is involved then start with one that
makes plots, and so on.  Obviously, there will be lots of changes to your starting point and
those are completely dependent on what the new module will do.  Yet, here are some steps
you are likely to encounter and some of them are required exactly as is and for others you
must adapt them to your module's specific purpose.

#. The first C code lines below the initial comments list a set of seven define statements
   that start with **THIS_MODULE_**. Most of them are fairly easy to update, such as stating
   the new module's name and purpose. Then there is the **THIS_MODULE_OPTIONS** parameter which
   is just a listing of all the common GMT options this module can use.  The next item is
   **THIS_MODULE_NEEDS** and it tells GMT if this module requires a region (e.g., via **-R**)
   and a projection (via **-J**).  Modules that must set a region to operate will either have
   the codes **R** (must take **-R**), **d** (can determine the region from given data sets),
   or **g** (can determine the region from a given grid).  There is also the mode **r** which
   means 'not normally required' but may be required depending on module options.  As an example,
   consider :doc:`pslegend` which does not need **-R** if **-Dx** is used but *does* need **-R** if
   other selections are made. For the same reason there is the **j** code for projections when
   it depends on other options whether **-J** is needed or not.  Most plot modules NEEDS will
   include **J**.  Finally, there is the very cryptic **THIS_MODULE_KEYS** setting. At this point,
   please read all the comments in the code for *GMT_Encode_Options* in gmt_api.c.  The **KEYS**
   are not used by the command line modules and only enters for some external interfaces, such
   as MATLAB and Julia.  You should have a discussion with those maintainers about what those
   **KEYS** should be.

#. Do some global replacements of strings: Replace the uppercase name of the template module
   with the new module uppercase name, and do the same for the lowercase name. This will ensure that
   your Ctrl structures and local static functions are named correctly (you may still need to
   delete some of them and add others).  All functions local to this module shall have names
   starting with the module name and an underscores (e.g., *grdinfo_report_tiles*) and they shall
   be stated as GMT_LOCAL instead of static (GMT_LOCAL is normally defined to mean static but
   in certain debugging cases we may wish to change that). The exceptions to this rule are the
   functions* New_Ctrl*, *Free_Ctrl*, *usage*, and *parse*; they are all static and have the
   same names in all modules.

#. After listing all local variables inside the main GMT_module function, all modules start with
   a section labeled *Standard module initialization and parsing*.  It is extremely unlikely
   you will need to make any changes in this section.  Ask for help if you think you do.

#. For style, argument checking, usage layout, use of built-in constants for the synopsis line,
   just follow the module template and draw inspiration from other modules.

#. Make sure you create a separate git branch and add your new code to that branch. New modules
   should never go into master and will have to undergo much scrutiny before the branch may
   be merged into master.

#. Try to compile GMT with your new module.  We automatically detect all the modules in the *src*
   directory (as well as supplemental source directories) and various needed "glue" functions
   are automatically created and compiled as part of the process.  You are likely to get some
   compilation errors.  Keep addressing them until the module compiles.  Running gmt *modulename*
   should print the full usage message.

#. The documentation of the module should be carefully designed.  You will need to add at least a
   module.rst file in the doc/rst/source directory that explains what the module does.  Again,
   start with a copy of another suitable module's RST file and make suitable changes.  Make sure
   you add some examples that others can try without getting data (e.g., use remote files as much
   as possible).

#. You must also edit the *modules.rst* file and add your new module and its purpose in two
   separate places.  Just search for a known module to see what is expected.  With these edits
   you should be able to build the full documentation and have your module show up under the
   Modules page.

#. Adding a new module to one of the official supplements pretty much follows the same steps.
   It is best to start from one of the other modules in that supplement since some of them
   have specific include files and setups.  Including the new supplemental module in the build
   process is automatic (done by CMake).

Compiling supplements
---------------------

For the purpose of this discussion, we will distinguish between several types of supplements:

#. Regular supplements included in the GMT distribution (e.g., *seis*, *spotter*).
#. External supplements not part of the GMT distribution but the developer wish to
   piggyback off the GMT build process instead of having a full-blown CMakeLists.txt setup.
   The  modules depend on (and include) gmt_dev.h, just like the core modules.
#. External supplements not part of the GMT distribution but has their own build setup.
   The  modules depend on (and include) gmt_dev.h, just like the core modules. One such
   example are the MB-System src/gmt tools.
#. External supplements not part of the GMT distribution but has their own build setup.
   THe modules only depend on (and include) gmt.h, the official GMT API.
