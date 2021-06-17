GMT on non-\ UNIX Platforms
===========================

Introduction
------------

While GMT was ported to non-\ UNIX systems such as Windows, it is
also true that one of the strengths of GMT lies its symbiotic
relationship with UNIX. We therefore recommend that GMT be installed
in a POSIX-compliant UNIX environment such as traditional
UNIX-systems, Linux, or Mac OS X. If abandoning your
non-\ UNIX operating system is not an option, consider one of these
solutions:

WINDOWS:
    Choose among these three possibilities:

    #. Windows Subsystem for Linux

    #. Install GMT under MinGW/MSYS2 (A collection of GNU utilities).

    #. Install GMT under Cygwin (A GNU port to Windows).

    #. Install GMT in Windows using Microsoft C/C++ or other
       compilers. Unlike the first two, this option will not provide you
       with any UNIX tools so you will be limited to what you can do
       with DOS batch files.


Windows Subsystem for Linux
---------------------------
If you're running Windows 10 you may take advantage of the Windows Subsystem for Linux.
This provides an easy way of setting up a functional Linux environment and required tools to build and use GMT.

From the Windows Subsystem for Linux Documentation:

    The Windows Subsystem for Linux lets developers run a GNU/Linux environment
    -- including most command-line tools, utilities, and applications
    -- directly on Windows, unmodified, without the overhead of a traditional virtual machine or dual-boot setup.

See `Windows Subsystem for Linux Documentation <https://docs.microsoft.com/en-us/windows/wsl/>`_ to get started.

Once WSL is installed, choose a distribution (e.g. Debian) and follow the `install <https://github.com/GenericMappingTools/gmt/blob/master/INSTALL.md#ubuntudebian>`_
instructions or `build from source <https://github.com/GenericMappingTools/gmt/blob/master/BUILDING.md>`_.

MINGW|MSYS2 and GMT
-------------------

Though one can install GMT natively using CMake, the simplest way of installing
under MINGW|MSYS2 is to just install the Windows binaries and use them from
the msys2 bash shell. As simple as that. Furthermore, GMT programs launch
faster here than on Cygwin so this is the recommended way of running
GMT on Windows. As one option, `Git for Windows <https://gitforwindows.org/>`_
can be easily installed and includes a bash emulator with MINGW|MSYS2.

Cygwin and GMT
--------------

Because GMT works best in conjugation with UNIX tools we suggest you
install GMT using the Cygwin product from Cygnus (now assimilated by
Redhat, Inc.). This free version works on any Windows version and it
comes with both the Bourne Again shell **bash** and the **tcsh**.
You also have access to most standard GNU development tools such as
compilers and text processing tools (**awk**, **grep**, **sed**,
etc.). Note that executables prepared for Windows will also run under Cygwin.

Follow the instructions on the Cygwin page on how to install the
package; note you must explicitly add all the development tool packages
(e.g., **gcc** etc) as the basic installation does not include them by
default. Once you are up and running under Cygwin, you may install
GMT  the same way you do under any other UNIX platform; our wiki
has instructions for packages you need to install first.

Finally, from Cygwin's User Guide: By default, no Cygwin program can
allocate more than 384 MB of memory (program and data). You should not
need to change this default in most circumstances. However, if you need
to use more real or virtual memory in your machine you may add an entry
in either the **HKEY_LOCAL_MACHINE** (to change the limit for all
users) or **HKEY_CURRENT_USER** (for just the current user) section of
the registry. Add the DWORD value **heap_chunk_in_mb** and set it to
the desired memory limit in decimal Mb. It is preferred to do this in
Cygwin using the **regtool** program included in the Cygwin package.
(For more information about **regtool** or the other Cygwin utilities,
see the Section called Cygwin Utilities in Chapter 3 of the Cygwin's
User Guide or use the help option of each utility.) You should always be
careful when using **regtool** since damaging your system registry can
result in an unusable system. This example sets the local machine memory
limit to 1024 Mb:

   ::

    regtool -i set /HKLM/Software/Cygnus\ Solutions/Cygwin/heap_chunk_in_mb 1024
    regtool -v list /HKLM/Software/Cygnus\ Solutions/Cygwin

For more installation details see the general README file.
