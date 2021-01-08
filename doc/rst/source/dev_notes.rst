.. index:: ! dev_notes

****************************
Documentation for Developers
****************************

This section contains low-level documentation for how some aspects of GMT have been
implemented or are in the planning-stages for design and implementation.  Users with
ideas for simplifications and general improvements are encouraged to open an issue and
share their thoughts with us. 

Long-options Implementation
---------------------------

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

which now becomes much easier to parse (for humans).
