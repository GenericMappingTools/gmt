.. _Long_Options:

Proposed Long-Options Implementation
====================================

What is the problem?
--------------------

Since GMT was first initiated, the modules have used a terse UNIX-inspired command-line
syntax::

    gmt blockmean -R0/5/0/6 -I1 my_table.txt > new_table.txt

It is not immediately obvious to a new user what the two options means.  UNIX tools
have the same problem, e.g., for sorting data into numerical order, one may use

::

    sort -g my_table.txt > sorted_table.txt

and nobody will know without reading the documentation for sort what **-g** means.  However, one
can now use the equivalent command

::

    sort --sort=general-numeric my_table.txt > sorted_table.txt

which is pretty self-explanatory.  In the case of GMT, similar long-options alternatives will be
implemented.  For instance, the above :doc:`/blockmean` command can also be written

::
    
    gmt blockmean --region=0/5/0/6 --increment=1 my_table.txt > new_table.txt

which now becomes much easier to parse (for humans). Unfortunately, GMT syntax is a bit
more complicated for many options.  Due to the inexorable growth of new capabilities,
many options have become more complex and may have optional *modifiers* appended to them.
Consider the common option **-i** that is used to specify which input columns the modules should read.  Not only
can it select *which* columns (e.g., **-i**\ 3,2,7-9), it allows optional modifiers that may be
repeated for each column (or column group) that handles basic data transformations.  For instance,
let us imagine that the above example needs column 3 to be used as is, but column 2 needs
to be converted by the log10 operator and columns 7-9 must be scaled by 10 and offset by -5.  In
standard (short) GMT syntax we would write

::

    -i3,2+l,7-9+s10+o-6

which only makes immediate sense to those who wrote the parser.  In contrast, for the
long-format syntax it will instead be

::

    --read-columns=3,2+log10,7-9+scale=10+offset=-5

which most users might be able to decipher.

Abstraction
-----------

So, there are several steps needed to implement this scheme across all GMT modules:

#. Build a set of long-option to short-option equivalences for the standard GMT
   :doc:`../std-opts`. This is only about 30 options.
#. Build sets of long-option to short-option equivalences for all the unique
   module options spread across ~150 modules (which includes the supplements).

Clearly, these translation tables will need to address not only the longer *names* for the options but
also the longer names for *modifiers*.  We can now be more abstract and state that a general
GMT short-format option actually follows a specific syntax::

    -option[directive][+modifier1[arg1]][+modifier2[arg2]][...]

where *option* and any *modifier* are single characters and there may be none, one,
or more modifiers following the initial option and the
optional *directive*.  As we saw in the case of **-i**, the sequence of "optional directive
followed by optional modifiers" may in fact be repeated by separating these sequences with
a comma.  The corresponding long-format syntax format is represented this way::

    -long-option[=directive][+modifier1[=arg1]][+modifier2[=arg2]][...]

where the key differences are

#. The *option* is a mnemonic word and not a single letter
#. Optional directives are appended after an equal sign
#. Optional *modifiers* use mnemonic words and not a single letter
#. Optional modifier arguments are appended after another equal sign


Implementation Details
----------------------

Common Options
~~~~~~~~~~~~~~

The approach taken has been to create a master translation table that relates the short and long
option syntax formats so that a function can be used to translate any general long-option
argument to the equivalent short-option argument.  That way, we only need to call this
function at the start of a module and do the replacement.  Then, the specific
parsers we already have for common and module options will work as is.  This design simplifies
the coding tremendously and only requires us to create the translation tables.
The approach has already been implemented and tested for the ~30 GMT Common Options and
developers can play with this by adding the compiler flag **-DUSE_COMMON_LONG_OPTIONS**
when building GMT.
The translations for the GMT common options are encapsulated in a single include file
(gmt_common_longoptions.h) that populates a *gmt_common_kw* structure and looks like this:

.. literalinclude:: ../../../../../src/gmt_common_longoptions.h
    :language: C
    :lines: 10-39

Here, *separator* is a comma if more than one repetition of the sequence is allowed, otherwise
it is 0.

Module Options
~~~~~~~~~~~~~~

For the ~150 individual modules it is probably not a good idea to introduce ~150 new
include files as was done for the common options above.  Instead, the translation structure
can be stored directly in the module C file.  For instance, the local *module_kw* structure
embedded in the blockmean.c module C code looks like this:

.. literalinclude:: ../../../../../src/blockmean.c
    :language: C
    :lines: 44-52

Given these translations we can execute long-format commands like this::

    gmt blockmean --region=0/20/10/56 --increment=1 --registration=pixel --select=sum data.txt > sums.txt

that will sum up all the values that fell inside each bin.
