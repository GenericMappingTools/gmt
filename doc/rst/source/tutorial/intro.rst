Introduction
============

Historical highlights
---------------------

The GMT system was initiated in late 1987 at Lamont-Doherty
Earth Observatory, Columbia University by graduate students Paul
Wessel and Walter H. F. Smith.  Version 1 was officially introduced
to Lamont scientists in July 1988.  GMT 1 migrated by word of mouth
(and tape) to other institutions in the United States, UK, Japan, and
France and attracted a small following.  Paul took a Post-doctoral
position at SOEST in December 1989 and continued the GMT development.
Version 2.0 was released with an article in EOS, October 1991, and
quickly spread worldwide.
Version 3.0 in 1993 which was released with another article in EOS
on August 15, 1995.  A major upgrade to GMT 4.0 took place in Oct 2004.
Finally, in 2013 we released the new GMT 5 series and we have updated this tutorial
to reflect the changes in style and syntax.  However, GMT 5 is generally
backwards compatible with GMT 4 syntax. In October 2019 we released the current
version GMT 6.
GMT is used by tens of thousands of users worldwide in a broad range of disciplines.

Philosophy
----------

GMT follows the UNIX philosophy in which complex tasks are broken
down into smaller and more manageable components.  Individual GMT
modules are small, easy to maintain, and can be used as any other
UNIX tool.  GMT is written in the ANSI C programming language
(very portable), is POSIX compliant, and is independent of
hardware constraints (e.g., memory).  GMT was deliberately written
for command-line usage, not a windows environment, in order to
maximize flexibility.  We standardized early on to use PostScript output
instead of other graphics formats.  Apart from the built-in support for
coastlines, GMT completely decouples data retrieval from the main
GMT modules.  GMT uses architecture-independent file formats.

GMT installation considerations
-------------------------------

See the `install guide <https://github.com/GenericMappingTools/gmt/blob/master/INSTALL.md>`_
for instructions and to make sure you have all required dependencies installed.
Alternatively, you can build GMT from source by following the
`building guide <https://github.com/GenericMappingTools/gmt/blob/master/BUILDING.md>`_.

In addition, we recommend access to any flavor of the UNIX operating system
(UNIX, Linux, macOS, Cygwin, MinGW, etc.).
We do not recommend using the DOS command window under Windows.
