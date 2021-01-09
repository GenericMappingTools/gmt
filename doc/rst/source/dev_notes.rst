.. index:: ! dev_notes

****************************
Documentation for Developers
****************************

This section contains low-level documentation for how some aspects of GMT have been
implemented or are in the planning-stages for design and implementation.  Users with
ideas for simplifications and general improvements are encouraged to open an issue and
share their thoughts with us. 

Proposed Long-options Implementation
------------------------------------

What is the problem?
~~~~~~~~~~~~~~~~~~~~

Since GMT was first initiated, the modules have used a terse UNIX-inspired command-line
syntax, such as::

    gmt blockmean -R0/5/0/6 -I1 my_table.txt > new_table.txt

It is not immediately obvious to the uninitiated what the two options means.  UNIX tools
have the same problem, e.g., for sorting data into numerical order, one may use::

    sort -g my_table.txt > sorted_table.txt

and nobody will know without reading the documentation what **-g** means.  However, one
can now use the equivalent command::

    sort --sort=general-numeric my_table.txt > sorted_table.txt

which is pretty self-explanatory.  In the case of GMT, similar long-options will be
implemented.  For instance, the above blockmean command can also be written::
    
    gmt blockmean --region=0/5/0/6 --increment=1 my_table.txt > new_table.txt

which now becomes much easier to parse (for humans). Unfortunately, GMT syntax is a bit
more complicated for many options.  Due to the inexorable growth of new capabilities,
many options have become more complex and may have optional *modifiers* appended to them.
Consider the option **-i** that is used to specify which input columns to read.  Not only
can it select which columns (e.g., **-i**\ 3,2,7-9), it allows modifiers that may be
repeated for each column or column group that handles basic translations.  For instance,
let us imagine that the above example needs column 3 to be used as is, but column 2 needs
to be converted to log10 and columns 7-9 shall be scaled by 10 and offset by -5.  In
standard GMT syntax we would write::

    -i3,2+l,7-9+s10+o-6

which only makes immediate sense to those who wrote that parser.  In long-format syntax it
will instead be:::

    --read-columns=3,2+log10,7-9+scale=10+offset=-5

which most users might be able to guess what is going on.

Abstraction
~~~~~~~~~~~

So, there are several steps to implement this scheme across all GMT modules:

#. Build a set of long-option to short-option equivalences for the standard GMT
   common options :doc:`std-opts`. This is only about 30 options.
#. Build sets of long-option to short-option equivalences for all the unique
   module options across ~150 modules (including supplements).

Clearly, these tables will need to address not only the longer *names* for the options but
also the longer names for *modifiers*.  We can now be more abstract and state that a general
GMT short-format option actually follows a specific syntax::

    -option[directive][+modifier1[arg1]][+modifier2[arg2]][...]

where there may be none, one, or more modifiers following the initial option and the
optional *directive*.  As we saw in the case of **-i**, the sequence of optional directive
followed by optional modifiers may in fact be repeated by separating these sequences with
a comma.  In long-format syntax the general format is::

    -long-option[=directive][+modifier1[=arg1]][+modifier2[=arg2]][...]

where the key differences are

#. The option is a mnemonic word and not a single letter
#. Optional directives are appended after an equal sign
#. Optional modifiers use mnemonic words and not a single letter
#. Optional modifier arguments are appended after another equal sign


Implementation
~~~~~~~~~~~~~~

The approach taken has been to create a master table that relates the short and long
option syntax items so that a function can be used to translate a general long-option
argument to the equivalent short-option argument.  That way, we only need to call this
function at the start of a module and do the replacement.  Then, the specific
parsers we have for common and module options will work as is.  This design simplifies
the coding tremendously and only requires us to create the translation tables.
The approach has already been implemented and tested for the ~30 GMT Common Options and
developers can play with this by adding the compiler flag **-DUSE_COMMON_LONG_OPTIONS**
when building GMT.
