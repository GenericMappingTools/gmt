#!/bin/sh
#
#	$Id: install_gmt.sh,v 1.19 2001-04-18 22:41:24 pwessel Exp $
#
#	Automatic installation of GMT version 3.4
#	Version for the Bourne shell (or compatible)
#
#	Paul Wessel
#	18-APR-2001
#--------------------------------------------------------------------------------
#	FUNCTIONS
#--------------------------------------------------------------------------------
# Get the functionality of echo -n
#--------------------------------------------------------------------------------
if [ x`echo -n` = x ]; then	# echo -n works
	echon()
	{
		echo -n "$*" 
	}
elif [ x`echo -e` = x ]; then	# echo -e works
	echon()
	{
		echo -e "$*"'\c'
	}
else				# echo with escapes better work
	echon()
	{
		echo "$*"'\c'
	}
fi
# Question poser, return answer
#--------------------------------------------------------------------------------
get_answer()
{
# Arg1 is question
echon "==> $1: " >&2
read answer
echo $answer
}
#--------------------------------------------------------------------------------
# Question poser with default answer, return answer
#--------------------------------------------------------------------------------
get_def_answer()
{
# Arg1 is question, Argv2 is default answer
echon "==> $1 [$2]: " >&2
read answer
echo $answer $2 | awk '{print $1}'
}
which_zip()
{
if [ `bzip2 -h 2>&1 | wc -l` -lt 10 ]; then	# Dont have bzip2
	if [ `gzip -h 2>&1 | wc -l` -lt 10 ]; then	# Dont have gzip
		echo "ERROR: Neither gzip or bzip2 installed - exits" >&2
		exit
	else
		use=gzip
	fi
else				# Try gzip
	use=bzip2
fi
echo $use
}
#--------------------------------------------------------------------------------
# INTERACTIVE GMT PARAMETER REQUEST SESSION
#--------------------------------------------------------------------------------
prep_gmt()
{
#--------------------------------------------------------------------------------
cat << EOF > gmt_install.ftp_site
1. SOEST, U of Hawaii [GMT Home], Honolulu, Hawaii, USA
2. NOAA, Lab for Satellite Altimetry, Silver Spring, Maryland, USA
3. IAG-USP, Dept of Geophysics, U. of Sao Paulo, BRAZIL
4. Inst for Geologi, U of Oslo, NORWAY
5. ISV, Hokkaido U, Sapporo, JAPAN
6. Charles Sturt U, Albury, AUSTRALIA
EOF
# Order (1-12) is 1:progs, 2:share, 3:high, 4:full, 5:suppl, 6:scripts
#		  7:ps, 8:pdf, 9:man, 10:web, 11:tut, 12:triangle
cat << EOF > gmt_install.ftp_bzsizes
0.55
3.7
8.6
28.8
0.53
3.1
3.1
7.3
0.09
1.6
1.0
0.09
EOF
cat << EOF > gmt_install.ftp_gzsizes
0.66
4.0
10.7
47.1
0.60
4.2
4.2
7.3
0.12
1.6
1.4
0.11
EOF
cat << EOF >&2
====>>>> Interactive installation of GMT <<<<====
		   Version 3.4
		  
We first need a questions and answer session to
determine how and where GMT is to be installed.
Then, when all parameters have been assembled,
we will run the installation (unless you chose
-n when starting this script).

EOF
topdir=`pwd`
os=`uname -s`
#--------------------------------------------------------------------------------
# See if user has defined NETCDFHOME, if so use it as default path
#--------------------------------------------------------------------------------

netcdf_path=${NETCDFHOME:-}

#--------------------------------------------------------------------------------
#	DETERMINE WHICH OF GZIP AND BZIP2 FILES TO USE
#--------------------------------------------------------------------------------

GMT_expand=`which_zip`
if [ $GMT_expand = "bzip2" ]; then	# Use bzip2
	suffix="bz2"
	expand="bzip2 -dc"
	sizes=gmt_install.ftp_bzsizes
	echo "+++ Will expand *.bz2 files made with bzip2 +++" >&2
elif [ $GMT_expand = "gzip" ]; then
	suffix="gz"
	expand="gzip -dc"
	sizes=gmt_install.ftp_gzsizes
cat << EOF >&2
+++ Will expand *.gz files make with gzip +++
    (Consider installing bzip2 (sourceware.cygnus.com/bzip2/index.html)
     since bzip2 files are considerably smaller\!)
EOF
fi

#--------------------------------------------------------------------------------
#	MAKE UTILITY
#--------------------------------------------------------------------------------

GMT_make=`get_def_answer "Enter make utility to use" "make"`

#--------------------------------------------------------------------------------
#	FTP MODE
#--------------------------------------------------------------------------------
cat << EOF >&2

If you are behind a firewall you are unlikely to have permission to initiate a
normal ftp session (which involves the server connecting back to the client).
If so, you may want to select passive ftp mode.

EOF
passive_ftp=`get_def_answer "Do you want passive ftp transmission (y/n)" "n"`

#--------------------------------------------------------------------------------
#	NETCDF SETUP
#--------------------------------------------------------------------------------

answer=`get_def_answer "Have you installed netcdf version 3.4 or later? (y/n)" "y"`
if [ $answer = "n" ]; then
	netcdf_path=""
	answer=`get_def_answer "Do you want me to ftp it for you? (y/n)" "y"`
	if [ $answer = "n" ]; then
		answer=`get_def_answer "Do you have netcdf.tar.Z (or .bz2, .gz) in $topdir? (y/n)" "y"`
		if [ $answer = "n" ]; then
			echo "Please ftp or install netcdf and then rerun install_gmt" >&2
			exit
		else
			netcdf_ftp=n
			ok=0
			if [ -f netcdf.tar.Z ]; then
				ok=1
			elif [ -f netcdf.tar.bz2 ] && [ $GMT_expand = "bzip2" ]; then
				ok=1
			elif [ -f netcdf.tar.gz ] && [ $GMT_expand = "gzip" ]; then
				ok=1
			fi
			if [ $ok -eq 0 ]; then
				echo "netcdf.tar.Z (or .bz2, .gz) not in $topdir, please ftp netcdf or have me do it" >&2
				exit
			fi
		fi
	else
		netcdf_ftp=y
	fi
	netcdf_install=y
else
	def=${NETCDFHOME:-/usr/local/netcdf-3.5.0}
	netcdf_path=`get_def_answer "Enter directory with netcdf lib and include" "$def"`
	netcdf_ftp=n
	netcdf_install=n
fi
	
#--------------------------------------------------------------------------------
#	GMT FTP SECTION
#--------------------------------------------------------------------------------

GMT_get_progs=d
GMT_get_share=d
GMT_get_scripts=d
GMT_get_suppl=d
GMT_get_ps=d
GMT_get_pdf=d
GMT_get_man=d
GMT_get_web=d
GMT_get_tut=d
GMT_get_high=d
GMT_get_full=d
GMT_get_triangle=d
GMT_triangle=n
GMT_ftpsite=1
GMT_ftp=`get_def_answer "Get any of the GMT version $VERSION archives via ftp? (y/n)" "y"`
if [ $GMT_ftp = "y" ]; then
	cat << EOF >&2

We offer $N_FTP_SITES different ftp sites.  Choose the one nearest
you in order to minimize net traffic and transmission times.
The sites are:

EOF
	cat gmt_install.ftp_site >&2
	echo " " >&2
	GMT_ftpsite=`get_def_answer "Enter your choice" "1"`
	if [ $GMT_ftpsite -le 0 ] || [ $GMT_ftpsite -gt $N_FTP_SITES ]; then
		GMT_ftpsite=1
		echo " Error in assigning site, using default site." >&2
	else
		echo " You selected site number $GMT_ftpsite:" >&2
		sed -n ${GMT_ftpsite}p gmt_install.ftp_site >&2
	fi
	echo " " >&2
	ftp_ip=`sed -n ${GMT_ftpsite}p gmt_install.ftp_ip`
	is_dns=`sed -n ${GMT_ftpsite}p gmt_install.ftp_dns`
	if [ $is_dns -eq 1 ]; then
		cat << EOF >&2
		
This anonymous ftp server $ftp_ip only accepts
connections from computers on the Internet that are registered
in the Domain Name System (DNS).  If you encounter a problem
connecting because your computer is not registered, please
either use a different computer that is registered or see your
computer systems administrator (or your site DNS coordinator)
to register your computer.

EOF
	fi

	echo " The first two archives are required for a minimal GMT install" >&2
	echo " " >&2

	size=`sed -n 1p $sizes`
	GMT_get_progs=`get_def_answer "Want the program source archive [$size Mb] (y/n)?" "y"`
	size=`sed -n 2p $sizes`
	GMT_get_share=`get_def_answer "Want the support data (coastlines) [$size Mb] (y/n)?" "y"`

	echo " " >&2
	echo " The next four archives are optional but recommended for a typical GMT install" >&2
	echo " " >&2

	size=`sed -n 6p $sizes`
	GMT_get_scripts=`get_def_answer "Want optional GMT example scripts and data [$size Mb] (y/n)?" "y"`
	size=`sed -n 5p $sizes`
	GMT_get_suppl=`get_def_answer "Want optional GMT supplemental programs [$size Mb] (y/n)?" "y"`
	size=`sed -n 7p $sizes`
	GMT_get_ps=`get_def_answer "Want optional GMT Documentation 1 (PS version) [$size Mb] (y/n)?" "y"`
	size=`sed -n 8p $sizes`
	GMT_get_pdf=`get_def_answer "Want optional GMT Documentation 2 (PDF version) [$size Mb] (y/n)?" "y"`
	size=`sed -n 9p $sizes`
	GMT_get_man=`get_def_answer "Want optional GMT Documentation 3 (Unix MAN) [$size Mb] (y/n)?" "y"`
	size=`sed -n 10p $sizes`
	GMT_get_web=`get_def_answer "Want optional GMT Web Documentation (HTML of all Docs) [$size Mb] (y/n)?" "y"`
	size=`sed -n 11p $sizes`
	GMT_get_tut=`get_def_answer "Want optional GMT tutorial data sets [$size Mb] (y/n)?" "y"`

	echo " " >&2
	echo " The next two archives contain bigger and more accurate coastline data:" >&2
	echo " " >&2

	size=`sed -n 3p $sizes`
	GMT_get_high=`get_def_answer "Want optional high resolution coastline data [$size Mb] (y/n)?" "y"`
	size=`sed -n 4p $sizes`
	GMT_get_full=`get_def_answer "Want optional full resolution coastline data [$size Mb] (y/n)?" "y"`

			     	
	echo " " >&2
	echo "GMT can use two different algorithms for Delauney triangulation." >&2
	echo " " >&2
	echo "   Shewchuck [1996]: Modern and very fast, copyrighted." >&2
	echo "   Watson [1982]   : Older and slower, public domain." >&2
	echo " " >&2
	echo "Because of the copyright, GMT uses Watson's routine by default." >&2
	echo " " >&2
	size=`sed -n 12p $sizes`
	GMT_triangle=`get_def_answer "Want optional Shewchuck's triangulation routine [$size Mb] (y/n)?" "n"`
	GMT_get_triangle=$GMT_triangle
else
	echo " " >&2
	echo "Since you do not want to ftp, the install procedure will" >&2
	echo "assume the compressed archives are in the current directory." >&2
fi
	
GMT_def="$topdir/GMT${VERSION}"
echo " " >&2
GMT_share=`get_def_answer "Directory for GMT data?" "$GMT_def/share"`
GMT_bin=`get_def_answer "Directory for GMT executables?" "$GMT_def/bin"`
GMT_lib=`get_def_answer "Directory for GMT linkable libraries?" "$GMT_def/lib"`
GMT_include=`get_def_answer "Directory for GMT include files?" "$GMT_def/include"`

cat << EOF >&2

Unix man pages are usually stored in /usr/man/manX, where X is
the relevant man section.  This is usually l for local.  Below,
you will be asked for X and the /usr/man part; the /manX will be
appended automatically, so do not answer /usr/man/manl

EOF
GMT_man=`get_def_answer "Directory for GMT man pages?" "$GMT_def/man"`
GMT_mansect=`get_def_answer "Enter Man page section for GMT man pages (1-9,l)" "l"`
GMT_web=`get_def_answer "Directory for GMT www pages?" "$GMT_def/www"`

cat << EOF >&2

At run-time, GMT uses the \$GMTHOME environmental parameter to
find the directory $GMT_share.
The name must NOT contain the trailing /share.
You may want to override the default if users will see a different
mount point or a symbolic link instead of the local directory.

EOF
def=`echo $GMT_share | sed -e 'sB/shareBBg'`
GMT_def=`get_def_answer "Enter default GMTHOME selection" "$def"`

cat << EOF >&2

The answer to the following question will modify the GMT defaults.
(You can always change your mind by editing share/gmt.conf)

EOF
answer=`get_def_answer "Do you prefer SI or US default values for GMT (s/u)" "s"`
if [ $answer = "s" ]; then
	GMT_si=y
else
	GMT_si=n
fi

cat << EOF >&2

The answer to the following question will modify the GMT defaults.
(You can always change your mind later by using gmtset)

PostScript (PS) files may contain commands to set paper size, pick
a specific paper tray, or ask for manual feed.  Encapsulated PS
files (EPS) are not intended for printers (but will print ok) and
can be included in other documents.  Both formats will preview
on most previwers (out-of-date Sun pageview is an exception).

EOF
answer=`get_def_answer "Do you prefer PS or EPS as default PostScript output (p/e)" "p"`
if [ $answer = "p" ]; then
	GMT_ps=y
else
	GMT_ps=n
fi

cat << EOF >&2

Building the GMT libraries as shared instead of static will
reduce executable sizes considerably.  GMT supports shared
libraries under Linux, SunOS, Solaris, IRIX, HPUX, and FreeBSD.
Under other systems you may have to manually configure macros
and determine what specific options to use with ld.

EOF
GMT_sharedlib=`get_def_answer "Try to make and use shared libraries? (y/n)" "n"`
cat << EOF >&2

If you have more than one C compiler you need to specify which,
otherwise just hit return to use the default compiler.

EOF
GMT_cc=`get_answer "Enter name of C compiler (include path if not in search path)"`

cat << EOF >&2

GMT passes information about previous GMT commands onto later
GMT commands via a hidden file (.gmtcommands).  To avoid that
this file is updated by more than one program at the same time
(e.g., when connecting two or more GMT programs with pipes) we
use POSIX advisory file locking on the file.  Apparently, some
versions of the Network File System (NFS) have not implemented
file locking properly.  We know this is the case with Linux
pre-2.4 kernels when mounting NFS disks from a Unix server.
If this is your case you should turn file locking OFF.

EOF
GMT_flock=`get_def_answer "Use POSIX Advisory File Locking in GMT (y/n)" "y"`

GMT_run_examples=`get_def_answer "Want to test GMT by running the 20 examples? (y/n)" "y"`
GMT_delete=`get_def_answer "Delete all tar files after install? (y/n)" "n"`

# export CONFIG_SHELL=`type sh | awk '{print $NF}'`

#--------------------------------------------------------------------------------
# Now do coastline archives
#--------------------------------------------------------------------------------

cat << EOF >&2

Normally, all coastline files are installed in ./GMT${VERSION}/share.
However, you can also place some of them in separate directories.
These dirs must exist or you must have write permission to make them.
If alternate directories are specified then a coastline.conf file will
be kept in ./GMT${VERSION}/share to contain the names of these directories.
NOTE:  Do not append the final /share as that is done automatically!

EOF

dir=${topdir}/GMT${VERSION}
GMT_dir_cli=`get_def_answer "Directory for int, low, and crude coastline files (without /share)" "$dir"`
if [ ! $GMT_get_high = n ]; then
	GMT_dir_high=`get_def_answer "Directory for high coastline files (without /share)" "$GMT_dir_cli"`
else
	GMT_dir_high=$GMT_dir_cli
fi
if [ ! $GMT_get_full = n ]; then
	GMT_dir_full=`get_def_answer "Directory for full coastline files (without /share)" "$GMT_dir_high"`
else
	GMT_dir_full=$GMT_dir_high
fi

GMT_suppl_cps=d
GMT_suppl_dbase=d
GMT_suppl_gshhs=d
GMT_suppl_imgsrc=d
GMT_suppl_meca=d
GMT_suppl_mex=d
GMT_suppl_mgg=d
GMT_suppl_misc=d
GMT_suppl_segyprogs=d
GMT_suppl_spotter=d
GMT_suppl_x2sys=d
GMT_suppl_x_system=d
GMT_suppl_xgrid=d
MATLAB=/usr/local/matlab
if [ ! $GMT_get_suppl = "n" ]; then

cat << EOF >&2

Several supplemental packages are available:

------------------------------------------------------------------------------
cps:       Encoding and decoding of Complete PostScript files for archiving
dbase:     Extracting data from NGDC DEM and other grids
gshhs:     Global Self-consistent Hierarchical High-resolution Shoreline extractor
imgsrc:    Extracting grids from global altimeter files (Sandwell/Smith)
meca:      Plotting special symbols in seismology and geodesy
mex:       Matlab interface for reading/writing GMT grdfiles (REQUIRES MATLAB)
mgg:       Programs for making, managing, and plotting MGD77 & .gmt data
misc:      Make posters on laserwriters and create bit-patterns
segyprogs: Plot SEGY seismic data files
spotter:   Plate tectonic backtracking and hotspotting
x2sys:     New (Generic) Track intersection (crossover) tools
x_system:  Old (MGG-specific) Track intersection (crossover) tools
xgrid:     An X11-based graphical editor for netCDF-based .grd files
------------------------------------------------------------------------------

EOF

	answer=`get_def_answer "Install any of the supplemental programs? (y/n/a(ll))?" "a"`
	if [ $answer = "a" ] || [ $answer = "n" ]; then
		y_or_n=n
		if [ $answer = "a" ]; then
			y_or_n=y
		fi
		GMT_suppl_cps=$y_or_n
		GMT_suppl_dbase=$y_or_n
		GMT_suppl_gshhs=$y_or_n
		GMT_suppl_imgsrc=$y_or_n
		GMT_suppl_meca=$y_or_n
		GMT_suppl_mex=$y_or_n
		GMT_suppl_mgg=$y_or_n
		GMT_suppl_misc=$y_or_n
		GMT_suppl_segyprogs=$y_or_n
		GMT_suppl_spotter=$y_or_n
		GMT_suppl_x2sys=$y_or_n
		GMT_suppl_x_system=$y_or_n
		GMT_suppl_xgrid=$y_or_n
	elif [ $answer = "y" ]; then
		GMT_suppl_cps=`get_def_answer "Install the cps supplemental package? (y/n)?" "y"`
		GMT_suppl_dbase=`get_def_answer "Install the dbase supplemental package? (y/n)?" "y"`
		GMT_suppl_gshhs=`get_def_answer "Install the gshhs supplemental package? (y/n)?" "y"`
		GMT_suppl_imgsrc=`get_def_answer "Install the imgsrc supplemental package? (y/n)?" "y"`
		GMT_suppl_meca=`get_def_answer "Install the meca supplemental package? (y/n)?" "y"`
		GMT_suppl_mex=`get_def_answer "Install the mex supplemental package? (y/n)?" "y"`
		GMT_suppl_mgg=`get_def_answer "Install the mgg supplemental package? (y/n)?" "y"`
		GMT_suppl_misc=`get_def_answer "Install the misc supplemental package? (y/n)?" "y"`
		GMT_suppl_segyprogs=`get_def_answer "Install the segyprogs supplemental package? (y/n)?" "y"`
		GMT_suppl_spotter=`get_def_answer "Install the spotter supplemental package? (y/n)?" "y"`
		GMT_suppl_x2sys=`get_def_answer "Install the x2sys supplemental package? (y/n)?" "y"`
		GMT_suppl_x_system=`get_def_answer "Install the x_system supplemental package? (y/n)?" "y"`
		GMT_suppl_xgrid=`get_def_answer "Install the xgrid supplemental package? (y/n)?" "y"`
	fi
	if [ $GMT_suppl_mex = "y" ]; then
		echo " " >&2
		echo "The mex supplement requires Matlab." >&2
		MATLAB=`get_def_answer "Enter MATLAB system directory" "$MATLAB"`
	fi
fi

file=`get_def_answer "Enter name of the parameter file that will now be created" "GMT${VERSION}.par"`

#--------------------------------------------------------------------------------
# SAVE SESSION SETTINGS TO INSTALL.PAR
#--------------------------------------------------------------------------------

cat << EOF > $file
# This file contains parameters needed by the install script
# for GMT Version ${VERSION} or later.  Give this parameter file
# as the argument to the install_gmt script and the whole
# installation process can be placed in the background.
# Default answers will be selected where none is given.
# You can edit the values, but do not remove definitions!
#
# This script was created by install_gmt on
#
EOF
date | awk '{printf "#\t%s\n", $0}' >> $file
cat << EOF >> $file
#
# Do NOT add any spaces around the = signs.  The
# file MUST conform to Bourne shell syntax
#---------------------------------------------
#       SYSTEM UTILITIES
#---------------------------------------------
GMT_expand=$GMT_expand
GMT_make=$GMT_make
#---------------------------------------------
#       NETCDF SECTION
#---------------------------------------------
netcdf_ftp=$netcdf_ftp
netcdf_install=$netcdf_install
netcdf_path=$netcdf_path
passive_ftp=$passive_ftp
#---------------------------------------------
#       GMT FTP SECTION
#---------------------------------------------
GMT_ftp=$GMT_ftp
GMT_ftpsite=$GMT_ftpsite
GMT_get_progs=$GMT_get_progs
GMT_get_share=$GMT_get_share
GMT_get_high=$GMT_get_high
GMT_get_full=$GMT_get_full
GMT_get_suppl=$GMT_get_suppl
GMT_get_scripts=$GMT_get_scripts
GMT_get_ps=$GMT_get_ps
GMT_get_pdf=$GMT_get_pdf
GMT_get_man=$GMT_get_man
GMT_get_web=$GMT_get_web
GMT_get_tut=$GMT_get_tut
GMT_get_triangle=$GMT_get_triangle
#---------------------------------------------
#       GMT SUPPLEMENTS SELECT SECTION
#---------------------------------------------
GMT_suppl_cps=$GMT_suppl_cps
GMT_suppl_dbase=$GMT_suppl_dbase
GMT_suppl_imgsrc=$GMT_suppl_imgsrc
GMT_suppl_gshhs=$GMT_suppl_gshhs
GMT_suppl_meca=$GMT_suppl_meca
GMT_suppl_mex=$GMT_suppl_mex
GMT_suppl_mgg=$GMT_suppl_mgg
GMT_suppl_misc=$GMT_suppl_misc
GMT_suppl_segyprogs=$GMT_suppl_segyprogs
GMT_suppl_spotter=$GMT_suppl_spotter
GMT_suppl_x2sys=$GMT_suppl_x2sys
GMT_suppl_x_system=$GMT_suppl_x_system
GMT_suppl_xgrid=$GMT_suppl_xgrid
#---------------------------------------------
#       GMT ENVIRONMENT SECTION
#---------------------------------------------
GMT_si=$GMT_si
GMT_ps=$GMT_ps
GMT_def=$GMT_def
GMT_share=$GMT_share
GMT_bin=$GMT_bin
GMT_lib=$GMT_lib
GMT_include=$GMT_include
GMT_man=$GMT_man
GMT_web=$GMT_web
GMT_dir_full=$GMT_dir_full
GMT_dir_high=$GMT_dir_high
GMT_dir_cli=$GMT_dir_cli
GMT_mansect=$GMT_mansect
#---------------------------------------------
#       COMPILING & LINKING SECTION
#---------------------------------------------
GMT_sharedlib=$GMT_sharedlib
GMT_cc=$GMT_cc
GMT_triangle=$GMT_triangle
GMT_flock=$GMT_flock
#---------------------------------------------
#       TEST & PRINT SECTION
#---------------------------------------------
GMT_run_examples=$GMT_run_examples
GMT_delete=$GMT_delete
MATDIR=$MATDIR
EOF

echo "Session parameters written to file $file" >&2
echo $file
}
#--------------------------------------------------------------------------------
# BACKGROUND INSTALLATION OF GMT FUNCTIONS
#--------------------------------------------------------------------------------
install_this_gmt()
# Get? File
{
	ok=1
	get_this=$1
	if [ -f GMT_$2.tar.$suffix ]; then
		this=GMT_$2.tar.$suffix
	elif [ -f GMT${VERSION}_$2.tar.$suffix ]; then
		this=GMT${VERSION}_$2.tar.$suffix
	else
		ok=0
	fi
	if [ $ok -eq 1 ] && [ $get_this != "n" ]; then	# File exists and we have not said no
		$expand $this | tar xvf -
	fi
}
install_triangle()
# Get? File
{
	get_this=$1
	if [ -f $2.tar.$suffix ] && [ $get_this != "n" ]; then	# File exists and we have not said no
		cd GMT${VERSION}
		$expand ../$2.tar.$suffix | tar xvf -
		cd ..
	fi
}
install_coast()
{
# Get? File dir
	get_this=$1
	file=$2
	dir=$3
	here=`pwd`
	ok=1
	done=0
	if [ -f GMT_${file}.tar.$suffix ]; then
		this=GMT_${file}.tar.$suffix
	elif [ -f GMT${VERSION}_${file}.tar.$suffix ]; then
		this=GMT${VERSION}_${file}.tar.$suffix
	else
		ok=0
	fi
	if [ $ok -eq 1 ] && [ $get_this != "n" ]; then	# File is present and wanted
		if [ ! -d $dir ]; then
			mkdir -p $dir
		fi				
		if [ ! -d $dir ]; then
			echo "Could not make the directory $dir - $this not untarred"
		else
			cd $dir
			$expand $here/$this | tar xvf -
			cd $here
			done=1
		fi
	fi
#	Special treatment for Companion CD-ROM with individual bzip2 files for high and full
#	that facilitates cross-platform install with Win32
	if [ -f GMT${file}c.bz2 ] && [ $get_this != "n" ] && [ $done -eq 0 ]; then	# File is present and wanted
		if [ ! -d $dir ]; then
			mkdir -p $dir
		fi				
		if [ ! -d $dir ]; then
			echo "Could not make the directory $dir - $this not untarred"
		else
			t=`echo $file | awk '{print substr($1,1,1)}'`
			echo "share/binned_GSHHS_${t}.cdf"
			$expand $here/GMT${file}c.bz2 > $dir/binned_GSHHS_${t}.cdf
			echo "share/binned_binned_${t}.cdf"
			$expand $here/GMT${file}r.bz2 > $dir/binned_river_${t}.cdf
			echo "share/binned_river_${t}.cdf"
			$expand $here/GMT${file}b.bz2 > $dir/binned_border_${t}.cdf
		fi
	fi
		
}
make_suppl()
# arg1=install? arg2=package
{
	get=$1
	pkg=$2
	if [ -d $pkg ] && [ $get != "n" ]; then
		echo "Installing the $pkg package."
		cd $pkg
		if [ $pkg = "mex" ] || [ $pkg = "xgrid" ]; then # Save makefiles from extermination
			\cp -f makefile makefile.copy
		fi
		$GMT_make spotless || exit
		if [ $pkg = "mex" ] || [ $pkg = "xgrid" ]; then # Restore makefiles
			\mv -f makefile.copy makefile
		fi
		$GMT_make all || exit
		if [ $write_bin -eq 1 ]; then
			$GMT_make install || ( echo "Problems during make install for $pgk - check manually later" )
			$GMT_make clean
		else
			echo "You do not have write permission to install binaries in $GMT_bin"
		fi
		cd ..
	fi
}
make_ftp_list()
{
# arg1=get arg2=file
	get_this=$1
	file=$2
	if [ $get_this = "y" ]; then
#		User has checked this one - first see if we already have it
		if [ -f GMT_${file}.tar.$suffix ]; then
			get=0
		elif [ -f GMT${VERSION}_${file}.tar.$suffix ]; then
			get=0
		else
			get=1
		fi
		if [ $get -eq 1 ]; then
			echo "get GMT_${file}.tar.$suffix" >> install_gmt.ftp_list
		fi
	fi
}
make_ftp_list2()
{
# arg1=get arg2=file
	get_this=$1
	file=$2
	if [ $get_this = "y" ]; then
#		User has checked this one - first see if we already have it
		if [ -f ${file}.tar.$suffix ]; then
			get=0
		else
			get=1
		fi
		if [ $get -eq 1 ]; then
			echo "get ${file}.tar.$suffix" >> install_gmt.ftp_list
		fi
	fi
}
#============================================================
#	START OF MAIN SCRIPT - INITIALIZATION OF PARAMETERS
#============================================================

trap "rm -f gmt_install.ftp_*; exit" 0 2 15
DIR=pub/gmt
VERSION=3.4
GMT=GMT
#--------------------------------------------------------------------------------
#	LISTING OF CURRENT FTP MIRROR SITES
#--------------------------------------------------------------------------------

N_FTP_SITES=6
cat << EOF > gmt_install.ftp_ip
gmt.soest.hawaii.edu
falcon.grdl.noaa.gov
ftp.iag.usp.br
ftp.geologi.uio.no
ftp.eos.hokudai.ac.jp
life.csu.edu.au
EOF

cat << EOF > gmt_install.ftp_dns
1
1
0
0
1
0
EOF
#--------------------------------------------------------------------------------

if [ $# -gt 0 ] && [ $1 = "-h" ]; then
	cat << EOF >&2
install_gmt 1.52 - Automatic installation of GMT ${VERSION}

GMT is installed in the background following the gathering
of installation parameters.  These parameters are obtained
in one of two ways:

(1) Via Internet: You may compose a parameter file using
    the form on the GMT home page (see the installation
    link under gmt.soest.hawaii.edu) and save the result
    of your request to a parameter file on your hard disk.
2)  Interactively: You may direct this script to start an
    interactive session which gathers information from you
    via a question-and-answer exchange and then saves your
    answers to a parameter file on your hard disk.
    
The parameter file is then passed on to the next stage which
carries out the installation without further interruptions.

Thus, two forms of the command are recognized:

install_gmt parameterfile [ &> logfile]  (for background install)
install_gmt [ -n ] [ &> logfile]	 (for interactive install)

The option -n means do NOT install, just gather the parameters.
Of course, there is also

      install_gmt.s -h			    (to display this message)
     
EOF
	exit
fi

do_install=1
if [ $# -eq 1 ] && [ $1 = "-n" ]; then	# Don not want to install yet
	do_install=0
	shift
fi

if [ $# -eq 1 ] && [ $1 != "-h" ]; then	# User gave a parameter file
	parfile=$1
	if [ ! -f $parfile ]; then
		echo "install_gmt: Parameter file $parfile not found" >&2
		exit
	fi
else			# We must run an interactive session first
	parfile=`prep_gmt`
	if [ $do_install -eq 0 ]; then	# Did not want to install yet
		exit
	fi
fi

#--------------------------------------------------------------------------------
#	INITIATE SETTINGS FROM PARAMETER FILE
#--------------------------------------------------------------------------------
# 
# Because arguments to the . command MUST be in the users PATH
# we must prepend ./ if the file is in the local directory since
# the user may not have . in his PATH
first=`echo $parfile | awk '{print substr($1,1,1)}'`
if [ $first = "/" ]; then	# absolute path OK
	. $parfile
else				# Local file, prepend ./
	. ./$parfile
fi

topdir=`pwd`
os=`uname -s`

if [ x"$GMT_expand" = x ]; then	# Was never set by user
	GMT_expand=`which_zip`
fi
if [ $GMT_expand = "bzip2" ]; then	# Use bzip2
	suffix="bz2"
	expand="bzip2 -dc"
	echo "+++ Will expand *.bz2 files made with bzip2 +++"
elif [ $GMT_expand = "gzip" ]; then
	suffix="gz"
	expand="gzip -dc"
	echo "+++ Will expand *.gz files make with gzip +++" >&2
	echo "    [Consider installing bzip2 (http://sourceware.cygnus.com/bzip2/index.html)" >&2
	echo "     since bzip2 files are considerably smaller\!]" >&2
fi

CONFIG_SHELL=`type sh | awk '{print $NF}'`
export CONFIG_SHELL

#--------------------------------------------------------------------------------
#	NETCDF SECTION
#--------------------------------------------------------------------------------

if [ ! x"$GMT_cc" = x ]; then
	CC=$GMT_cc
	export CC
fi

if [ $netcdf_install = "y" ]; then
	if [ $netcdf_ftp = "y" ]; then

		cd $topdir
	
#		Set-up ftp command
  
		echo "user anonymous $USER@" > $$
		if [ $passive_ftp = "y" ]; then
			echo "passive" >> $$
			echo "quote pasv" >> $$
		fi
		echo "cd pub/netcdf" >> $$
		echo "binary" >> $$
		echo "get netcdf.tar.Z" >> $$
		echo "quit" >> $$
		echo " " >> $$

#		Get the file

		echo "Getting netcdf by anonymous ftp (be patient)..." >&2
		before=`du -sk . | cut -f1`
		ftp -dn unidata.ucar.edu < $$ || ( echo "ftp failed - try again later" >&2; exit )
		after=`du -sk . | cut -f1`
		newstuff=`echo $before $after | awk '{print $2 - $1}'`
		echo "Got $newstuff kb ... done" >&2
		rm -f $$
	fi

	if [ -f netcdf.tar.Z ]; then
		zcat netcdf.tar.Z | tar xvf -
	elif [ -f netcdf.tar.bz2 ] && [ $GMT_expand = "bzip2" ]; then
		$expand netcdf.tar.$suffix | tar xvf -
	elif [ -f netcdf.tar.gz ] && [ $GMT_expand = "gzip" ]; then
		$expand netcdf.tar.$suffix | tar xvf -
	else
		echo "?? netcdf.tar.{Z,bz2,gz} not found - must abort !!"
		exit
	fi
	
	n_version=`cat netcdf*/src/VERSION | sort -r | head -1`
	cd netcdf-${n_version}/src

#	Interix/MacOS fix for bad lex which creates an #include statement for values.h which
#	which does not exist.  We create an empty values.h file in the ncgen directory:

	if [ $os = "Windows_NT" ] || [ $os = "Rhapsody" ] || [ $os = "Darwin" ]; then
		touch ncgen/values.h
	fi
	netcdf_path=${netcdf_path:-$topdir/netcdf-${n_version}}
	if [ $os = "Linux" ]; then
		DEFINES="-Df2cFortran"
		export DEFINES
	fi
	rm -f config.{cache,log,status}
	if [ $os = "Darwin" ]; then	# Get special versions of config.* for MacOS X
		cp /usr/libexec/config.* .
		host="--host=powerpc-apple-Darwin"
	else
		host=
	fi
	./configure $host --prefix=$netcdf_path
	$GMT_make || exit
	$GMT_make test || exit
	$GMT_make install || exit
	$GMT_make clean || exit
	if [ $os = "Windows_NT" ] || [ $os = "Rhapsody" ] || [ $os = "Darwin" ]; then	# Lord giveth, lord taketh away
		rm -f ncgen/values.h
	fi
	cd ../..
	if [ $GMT_delete = "y" ]; then
		rm -f netcdf.tar.Z
	fi
fi

if [ x"$netcdf_path" = x ]; then	# Not explicitly set, must assign it
	if [ ! x"$NETCDFHOME" = x ]; then	# Good, used an environmental variable for it
                netcdf_path=$NETCDFHOME
        elif [ $netcdf_ftp = "n" ]; then	# First see if it was already installed in $topdir
                netcdf_path=$topdir/netcdf-3.?
                if [ x"$netcdf_path" = x ]; then	# No, give default place
                	echo "install_gmt: No path for netcdf provided - default is /usr/local/netcdf" >&2
     			netcdf_path="/usr/local/netcdf"
		fi
                NETCDFHOME=$netcdf_path
                export NETCDFHOME
        fi
else
        NETCDFHOME=$netcdf_path
        export NETCDFHOME
fi

#--------------------------------------------------------------------------------
#	GMT FTP SECTION
#--------------------------------------------------------------------------------

cd $topdir
if [ $GMT_ftp = "y" ]; then

	if [ $GMT_ftpsite -le 0 ] || [ $GMT_ftpsite -gt $N_FTP_SITES ]; then
		GMT_ftpsite=1
		echo " Error in assigning site, use default site $GMT_ftpsite" >&2
	fi
	ftp_ip=`sed -n ${GMT_ftpsite}p gmt_install.ftp_ip`
	is_dns=`sed -n ${GMT_ftpsite}p gmt_install.ftp_dns`

#	Set-up ftp command
  
	echo "user anonymous $USER@" > install_gmt.ftp_list
	if [ $passive_ftp = "y" ]; then
		echo "passive" >> install_gmt.ftp_list
		echo "quote pasv" >> install_gmt.ftp_list
	fi
	echo "cd $DIR" >> install_gmt.ftp_list
	echo "binary" >> install_gmt.ftp_list
	make_ftp_list $GMT_get_progs progs
	make_ftp_list $GMT_get_share share
	make_ftp_list $GMT_get_high high
	make_ftp_list $GMT_get_full full
	make_ftp_list $GMT_get_suppl suppl
	make_ftp_list $GMT_get_scripts scripts
	make_ftp_list $GMT_get_ps ps
	make_ftp_list $GMT_get_pdf pdf
	make_ftp_list $GMT_get_man man
	make_ftp_list $GMT_get_web web
	make_ftp_list $GMT_get_tut tut
	make_ftp_list2 $GMT_get_triangle triangle
	echo "quit" >> install_gmt.ftp_list
	echo " " >> install_gmt.ftp_list

#	Get the files

	echo "Getting GMT by anonymous ftp from $ftp_ip (be patient)..." >&2

	before=`du -sk . | cut -f1`
	ftp -dn $ftp_ip < install_gmt.ftp_list || ( echo "fpt failed - try again later >&2"; exit )
	after=`du -sk . | cut -f1`
	rm -f install_gmt.ftp_list
	newstuff=`echo $before $after | awk '{print $2 - $1}'`
	echo "Got $newstuff kb ... done" >&2
fi

#--------------------------------------------------------------------------------
# First install source code and documentation
#--------------------------------------------------------------------------------

install_this_gmt $GMT_get_progs progs
install_this_gmt $GMT_get_suppl suppl
install_this_gmt $GMT_get_scripts scripts
install_this_gmt $GMT_get_ps ps
install_this_gmt $GMT_get_pdf pdf
install_this_gmt $GMT_get_man man
install_this_gmt $GMT_get_web web
install_this_gmt $GMT_get_tut tut
install_triangle $GMT_get_triangle triangle

#--------------------------------------------------------------------------------
# Now do coastline archives
#--------------------------------------------------------------------------------

dir=${topdir}/GMT${VERSION}
GMT_dir_full=${GMT_dir_full:-$dir}
if [ $GMT_dir_full != $dir ]; then
	echo $GMT_dir_full >> $$.coast
fi

GMT_dir_high=${GMT_dir_high:-$dir}
if [ $GMT_dir_high != $dir ]; then
	echo $GMT_dir_high >> $$.coast
fi
GMT_dir_cli=${GMT_dir_cli:-$dir}
if [ $GMT_dir_cli != $dir ]; then
	echo $GMT_dir_cli >> $$.coast
fi

install_coast $GMT_get_share share $GMT_dir_cli
install_coast $GMT_get_high  high  $GMT_dir_high
install_coast $GMT_get_full  full  $GMT_dir_full

if [ -f $$.coast ]; then	# Install coastline.conf file
	echo "# GMT Coastline Path Configuration File" > $topdir/GMT${VERSION}/share/coastline.conf
	echo "" >> $topdir/GMT${VERSION}/share/coastline.conf
	sort -u $$.coast >> $topdir/GMT${VERSION}/share/coastline.conf
	echo "$topdir/GMT${VERSION}/share/coastline.conf initialized" >&2
	rm -f $$.coast
fi

echo " " >&2
echo "write privileges on all files in GMT${VERSION} ..." >&2
cd GMT${VERSION}
chmod -R +w .
cd ..
echo "Done" >&2
echo " " >&2

#--------------------------------------------------------------------------------
#	GMT INSTALLATION PREPARATIONS
#--------------------------------------------------------------------------------

cd $topdir
cd GMT${VERSION}
here=`pwd`

# If we got here via a parameter file that had blank answers
# we need to provide the default values here

GMT_share=${GMT_share:-$here/share}
dir=`echo $GMT_share | sed -e 'sB/shareBBg'`
GMT_def=${GMT_def:-$dir}

# Are we allowed to write in $GMT_share?

if [ -w $dir ]; then
	write_share=1
else
	write_share=0
fi

GMT_bin=${GMT_bin:-$here/bin}
GMT_lib=${GMT_lib:-$here/lib}
GMT_include=${GMT_include:-$here/include}

# Are we allowed to write in $GMT_bin?

dir=`echo $GMT_bin | sed -e 'sB/binBBg'`
if [ -w $dir ]; then
	write_bin=1
else
	write_bin=0
fi

GMT_man=${GMT_man:-$here/man}

# Are we allowed to write in $GMT_man?

dir=`echo $GMT_man | sed -e 'sB/manBBg'`
if [ -w $dir ]; then
	write_man=1
else
	write_man=0
fi
GMT_web=${GMT_web:-$here/www}

# Are we allowed to write in $GMT_web?

dir=`echo $GMT_web | sed -e 'sB/webBBg'`
if [ -w $dir ]; then
	write_web=1
else
	write_web=0
fi

#--------------------------------------------------------------------------------
#	CONFIGURE PREPARATION
#--------------------------------------------------------------------------------

if [ $GMT_si = "y" ]; then
	enable_us=
else
	enable_us=--enable-US
fi
if [ $GMT_ps = "y" ]; then
	enable_eps=
else
	enable_eps=--enable-eps
fi
if [ $GMT_flock = "y" ]; then
	disable_flock=
else
	disable_flock=--disable-flock
fi
if [ $GMT_triangle = "y" ]; then
	enable_triangle=--enable-triangle
else
	enable_triangle=
fi

if [ $GMT_sharedlib = "y" ]; then
	enable_shared=--enable-shared
else
	enable_shared=
fi

if [ ! x"$MATDIR" = x ]; then	# MATDIR is set
	MATLAB=$MATDIR
	export MATLAB
fi

#--------------------------------------------------------------------------------
#	GMT installation commences here
#--------------------------------------------------------------------------------

cat << EOF >&2

---> Begin GMT $VERSION installation <---

---> Run configure to create makegmt.macros and gmt_notposix.h

EOF

# Clean out old cached values

rm -f config.{cache,log,status}

# Clean out old makegmt.macros etc if present

if [ -f src/makegmt.macros ]; then
	echo '---> Clean out old executables, *.o, *.a, gmt_nan.h, and makegmt.macros' >&2
	$GMT_make spotless || exit
fi
	
if [ $os = "Darwin" ]; then	# Get special versions of config.* for MacOS X
	cp /usr/libexec/config.* .
	host="--host=powerpc-apple-Darwin"
else
	host=
fi
./configure $host --prefix=$GMT_def --bindir=$GMT_bin --libdir=$GMT_lib --includedir=$GMT_include $enable_us \
  $enable_eps $disable_flock $enable_shared $enable_triangle --mandir=$GMT_man --enable-mansect=$GMT_mansect --enable-www=$GMT_web --datadir=$GMT_share

if [ -f .gmtconfigure ]; then
	cat .gmtconfigure
	rm -f .gmtconfigure
fi

# OK, descend into src directory

cd src

echo "---> Create gmt_nan.h" >&2

$GMT_make init || exit

echo "---> Make all" >&2

$GMT_make all || exit

if [ $write_bin -eq 1 ]; then
	echo "---> Make install" >&2

	$GMT_make install || exit
else
	echo "You do not have write permission to make $GMT_bin" >&2
fi

cd ..

#--------------------------------------------------------------------------------
# RUN EXAMPLES
#--------------------------------------------------------------------------------

# Run examples with /src as binary path and /share as GMTHOME in case the user did
# not have permission to place files in GMT_share and GMT_bin

if [ -d examples ]; then
	if [ $GMT_run_examples = "y" ]; then
		$GMT_make run-examples || exit
	fi
fi


#--------------------------------------------------------------------------------
# INSTALL SUPPLEMENTAL PROGRAMS
#--------------------------------------------------------------------------------

if [ -d src/cps ]; then
	cd src
	make_suppl $GMT_suppl_cps cps
	make_suppl $GMT_suppl_dbase dbase
	make_suppl $GMT_suppl_gshhs gshhs
	make_suppl $GMT_suppl_imgsrc imgsrc
	make_suppl $GMT_suppl_meca meca
	make_suppl $GMT_suppl_mex mex
	make_suppl $GMT_suppl_mgg mgg
	make_suppl $GMT_suppl_misc misc
	make_suppl $GMT_suppl_segyprogs segyprogs
	make_suppl $GMT_suppl_spotter spotter
	make_suppl $GMT_suppl_x2sys x2sys
	make_suppl $GMT_suppl_x_system x_system
	make_suppl $GMT_suppl_xgrid xgrid
	cd ..
fi

#--------------------------------------------------------------------------------
# INSTALL LIB DIRECTORY
#--------------------------------------------------------------------------------

if [ $write_share -eq 1 ]; then
	$GMT_make install-data || exit
fi

#--------------------------------------------------------------------------------
# INSTALL MAN PAGES
#--------------------------------------------------------------------------------

if [ $write_man -eq 1 ]; then
	if [ -d man/manl ]; then
		$GMT_make install-man || exit
		echo "All users must include $GMT_man in their MANPATH" >&2
	else
		echo "GMT Man pages not installed" >&2
		GMT_man=n
	fi
else
	echo "You do not have write permission to make $GMT_man" >&2
fi


#--------------------------------------------------------------------------------
# INSTALL WWW PAGES
#--------------------------------------------------------------------------------

if [ $write_web -eq 1 ]; then
	if [ -d www ]; then
		$GMT_make install-www || exit
		echo "All users should add $GMT_web/gmt/gmt_services.html to their browser bookmarks" >&2
	fi
else
	echo "You do not have write permission to create $GMT_web" >&2
fi

cd $here/src
if [ $write_bin -eq 1 ]; then
	$GMT_make clean || ( echo "Problems during make clean - check manually" >&2 )
else
	echo "Manually do the final installs as another user (root?)" >&2
	echo "Go to the main GMT directory and say:" >&2
	echo "make install install-suppl clean" >&2
fi
if [ $write_share -eq 0 ]; then
	echo "Manually do the coastline install as another user (root?)" >&2
	echo "Go to the main GMT directory and say:" >&2
	echo "make install-data" >&2
fi
if [ $write_man -eq 0 ]; then
	echo "Manually do the man page install as another user (root?)" >&2
	echo "Go to the main GMT directory and say:" >&2
	echo "make install-man" >&2
fi
if [ $write_web = 0 ]; then
	echo "Manually do the www page install as another user (root?)" >&2
	echo "Go to the main GMT directory and say:" >&2
	echo "make install-www" >&2
fi

if [ $GMT_delete = "y" ]; then
	rm -f GMT*.tar.$suffix triangle.tar.$suffix
	if [ -f GMTfullc.bz2 ]; then	# Special files copied from CD-ROM
		rm -f GMTfull?.bz2 GMThigh?.bz2
	fi
fi

dir=`echo $GMT_share | sed -e 'sB/shareBBg'`
cat << EOF >&2
GMT installation complete. Remember to set these:

-----------------------------------------------------------------------
For csh or tcsh users:
setenv GMTHOME $dir
set path=($GMT_bin \$path)
For sh or bash users:
export GMTHOME=$dir
export PATH=$GMT_bin:\$PATH
For all users:
EOF
if [ ! x"$GMT_man" = x ]; then
	echo "Add $GMT_man to MANPATH" >&2
fi
if [ ! x"$GMT_web" = x ]; then
	echo "Add $GMT_web/gmt/gmt_services.html as browser bookmark" >&2
fi
echo "-----------------------------------------------------------------------" >&2
cd $topdir
