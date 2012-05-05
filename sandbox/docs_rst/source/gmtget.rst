######
gmtget
######


gmtget - Get individual **GMT** default parameters

`Synopsis <#toc1>`_
-------------------

**gmtget** [ **-G**\ *defaultsfile* ] [ **-L** ] *PARAMETER1* [
*PARAMETER2* *PARAMETER3* ... ]

`Description <#toc2>`_
----------------------

**gmtget** will list the value of one or more **GMT** default
parameters.

`Required Arguments <#toc3>`_
-----------------------------

PARAMETER
    Provide one or several parameters of interest. The current value of
    those parameters will be writen to *stdout*. For a complete listing
    of available parameters and their meaning, see the **gmt.conf** man
    page.

`Optional Arguments <#toc4>`_
-----------------------------

**-G**\ *defaultsfile*
    Name of specific **gmt.conf** file to read [Default looks first in
    current directory, then in your home directory, then in ~/.gmt and
    finally in the system defaults].
**-L**
    Return the values of the parameters on separate lines [Default
    returns all selected parameter values on one line separated by
    spaces]

`Example <#toc5>`_
------------------

To list the value of the parameter PS\_COMMENTS:

**gmtget** PS\_COMMENTS To get both the values of the parameter
GRID\_CROSS\_SIZE\_PRIMARY and GRID\_CROSS\_SIZE\_SECONDARY on one line,
try

**gmtget** GRID\_CROSS\_SIZE\_PRIMARY GRID\_CROSS\_SIZE\_SECONDARY

`See Also <#toc6>`_
-------------------

`*gmt*\ <gmt.html>`_ , `*gmt.conf*\ <gmt.conf.html>`_ ,
`*gmtset*\ <gmtset.html>`_

