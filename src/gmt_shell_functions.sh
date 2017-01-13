#!/bin/bash
#	$Id$
#
# These functions can be used from any sh/bash script by specifying
# . gmt_shell_functions.sh
# in your script. Placing it in .bashrc makes the functions available
# on the command line as well.  See documentation for usage.
#
# Note: Several of the original functions are now obsolete due to
# new functionality in GMT.  These are still available for use
# but no longer documented and are placed at the end of this file
# under the OBSOLETE tag.

#----GMT SHELL FUNCTIONS--------------------
#	Creates a unique temp directory and points GMT_TMPDIR to it
gmt_init_tmpdir () {
	export GMT_TMPDIR=`mktemp -d ${TMPDIR:-/tmp}/gmt.XXXXXX`
}

#	Remove the temp directory created by gmt_init_tmpdir
gmt_remove_tmpdir () {
	rm -rf $GMT_TMPDIR
	unset GMT_TMPDIR
}

#	Remove all files and directories in which the current process number is part of the file name
gmt_cleanup() {
	rm -rf *$$*
	if [ $# -eq 1 ]; then
		rm -rf ${1}*
	fi
}

#	Send a message to stderr
gmt_message() {
	echo "$*" >&2
}

#	Print a message to stderr and exit
gmt_abort() {
	echo "$*" >&2
	exit
}

#	Returns the number of fields or arguments
gmt_get_nfields() {
	echo $* | awk '{print NF}'
}
#	Same with backwards compatible name...
gmt_nfields() {
	echo $* | awk '{print NF}'
}

#	Returns the given field (arg 1) in current record (arg 2)
#	Must pass arg 2 inside double quotes to preserve it as one item
gmt_get_field() {
	echo $2 | cut -f$1 -d ' '
}

#	Return w/e/s/n from given table file(s)
#	May also add -Idx/dy to round off answer
gmt_get_region() {
	printf "%s/%s/%s/%s\n" `gmt info -C $* | cut -d'	' -f1-4`
}

#	Return the w/e/s/n from the header in grd file
gmt_get_gridregion() {
	printf "%s/%s/%s/%s\n" `gmt grdinfo -C $* | cut -d'	' -f2-5`
}

# Make output PostScript file name based on script base name
gmt_set_psfile() {
	echo `basename $1 '.sh'`.ps
}

# Make output PDF file name based on script base name
gmt_set_pdffile() {
	echo `basename $1 '.sh'`.pdf
}

# For animations: Create a lexically increasing file namestem (no extension) based on prefix and frame number
# i.e., prefix_######
gmt_set_framename() {
	echo $1 $2 | awk '{printf "%s_%06d\n", $1, $2}'
}

# For animations: Increment frame counter by one
gmt_set_framenext() {
	echo $(($1 + 1))
}

# For KMZ: Package a bunch of *.kml [and *.png] into a single kmz
gmt_build_kmz() {
	if [ $# -eq 0 ]; then
		cat << EOF >&2
gmt_build_kmz - Create a single KMZ file from many KML files

usage: gmt_build_kmz -p <prefix> [-r] *.kml [*.png]
	*.kml: The KML files you want to include in the KML file.
	   If these link to local PNG files then list those too.
	-p Specify a prefix for naming the <prefix>.kmz file
	-r Remove all provided files after KMZ is built [leave alone]
EOF
		return
	fi
	if [ ! "X$1" = "X-p" ]; then
		echo "gmt_build_kmz:  Requires -p <prefix> as first argument" >&2
		return
	fi
	remove=0
	name=$2 ; shift ; shift;
	if [ "X$1" = "X-r" ]; then
		remove=1
		shift
	fi
	mkdir -p kml; mv -f $* kml
	cat <<- EOF > doc.kml
	<?xml version="1.0" encoding="UTF-8"?>
	<kml xmlns="http://www.opengis.net/kml/2.2">
		<Document>
	EOF
	ls kml/*.kml > /tmp/$$.lis
	while read file; do
		cat <<- EOF >> doc.kml
			<NetworkLink>
	        	<name>$file</name>
	        	<Link>
				<href> $file </href>
			</Link>
			</NetworkLink>
		EOF
	done < /tmp/$$.lis
	cat <<- EOF >> doc.kml
		</Document>
	</kml>
	EOF
	zip -rq9 $name.kmz doc.kml kml
	if [ $remove -eq 0 ]; then
		mv -f kml/* ..
	fi
	rm -rf kml doc.kml /tmp/$$.lis
}

# For animations: Build animated gif from stills
gmt_build_gif() {
	if [ $# -eq 0 ]; then
		cat << EOF >&2
gmt_build_gif - Process stills to animated gif with convert

usage: gmt_build_gif [-d <directory>] [-l <loop>] [-r <delay>] <prefix>
	<prefix> is the prefix of the still images and the resulting movie
	-d Specify path to of directory with the stills [current directory]
	-l Specify number of times to loop, 0 for continous [0]
	-r Set delay time between images in millisecond [10]
	-n Dry-run.  Do not run convert command, just print it
EOF
		return
	fi
	missing=`which -s ${GRAPHICSMAGICK-gm}`
	if [ $missing -eq 1 ]; then
		echo "gmt_build_gif: Cannot find gm in your path - exiting" >&2
		return
	fi
	delay=24; dir=.; loop=0; dryrun=0
	while [ $# -ne 1 ]; do
		case "$1" in
		"-d") dir=$2 ; shift ;;
		"-l") loop=$2 ; shift ;;
		"-n") dryrun=1 ;;
		"-r") delay=$2 ; shift ;;
		*) echo "gmt_build_gif:  No such option ($1)" >&2
		    ;;
		esac
		shift
	done
	if [ $dryrun -eq 1 ]; then
		cat <<- EOF
${GRAPHICSMAGICK-gm} convert -delay $delay -loop $loop +dither "$dir/${1}_*.*" ${1}.gif
		EOF
	else
		${GRAPHICSMAGICK-gm} convert -delay $delay -loop $loop +dither "$dir/${1}_*.*" ${1}.gif
	fi
}

# For animations: Build a m4v movie from stills
gmt_build_movie() {
	if [ $# -eq 0 ]; then
		cat << EOF >&2
gmt_build_movie - Process stills to m4v movie with ffmpeg
	Note: M4V Requires images to have an even pixel width

usage: gmt_build_movie [-d <directory>] [-n] [-r <rate>] [-v] <prefix>
	<prefix> is the prefix of the still images and the resulting movie
	-d Specify path to of directory with the stills [current directory]
	-r Set frame rate in images per second [24]
	-n Dry-run.  Do not run ffmpeg command, just print it
	-v Verbose.  Give progress messages
EOF
		return
	fi
	missing=`which -s ffmpeg`
	if [ $missing -eq 1 ]; then
		echo "gmt_build_movie: Cannot find ffmpeg in your path - exiting" >&2
		return
	fi
	rate=24; dir=.; dryrun=0; blabber=quiet
	while [ $# -ne 1 ]; do
		case "$1" in
		"-d") dir=$2 ; shift ;;
		"-n") dryrun=1 ;;
		"-r") rate=$2 ; shift ;;
		"-v") blabber=verbose ;;
		*) echo "gmt_build_movie:  No such option ($1)" >&2
		    ;;
		esac
		shift
	done
	if [ $dryrun -eq 1 ]; then
		cat <<- EOF
ffmpeg -loglevel $blabber -f image2 -pattern_type glob -framerate $rate -y -i "$dir/$1_*.*" -pix_fmt yuv420p ${1}.m4v
		EOF
	else
		ffmpeg -loglevel $blabber -f image2 -pattern_type glob -framerate $rate -y -i "$dir/$1_*.*" -pix_fmt yuv420p ${1}.m4v
	fi
}

# For spreading numerous commands across many CPUs in clusters of N lines
gmt_launch_jobs() {
	# gmt_launch_jobs -c <n_cpu> -j <nlines_per_cluster> <commandfile>
	# Split the non-comment records in <commandfile> into <n_cpu> files
	# so that number of lines per file is a multiple of <nlines_per_cluster>.
	n_cpu=`gmt --show-cores`
	if [ $# -eq 0 ]; then
		cat << EOF >&2
gmt_launch_jobs - Run chunks of commands in parallel

usage: gmt_launch_jobs [-c <n_cpu>] [-l <nlines_per_cluster>] [-n] [-v] [-w] <commandfile>
	<commandfile> is a file with a list of all the commands
	-c Specify how many separate cores to use [$n_cpu]
	-l Specify how many lines constitute one job cluster [1]
	-n Dry-run.  Do not launch jobs but leave core scripts as /tmp/gmt_launch_jobs.##.sh
	-r Remove core scripts when the jobs complete
	-v Verbose.  Give progress messages
	-w Wait for completion of all core jobs before exiting
EOF
		return
	fi
	n_lines=1; do_wait=0; do_remove=0; dryrun=0; blabber=0
	while [ $# -ne 1 ]; do
		case "$1" in
		"-c") n_cpu=$2 ; shift ;;
		"-l") n_lines=$2 ; shift ;;
		"-n") dryrun=1 ;;
		"-r") do_remove=1 ;;
		"-v") blabber=1 ;;
		"-w") do_wait=1 ;;
		*) echo "gmt_launch_jobs:  No such option ($1)" >&2
		    ;;
		esac
		shift
	done
	egrep -v '^#|^$' $1 > /tmp/$$.sh
	nL=`wc -l /tmp/$$.sh | awk '{printf "%d\n", $1}'`
	n_chunks=`gmt math -Q $nL $n_lines DIV =`
	bad=`gmt math -Q $n_chunks DUP RINT SUB ABS 1e-10 GT =`
	if [ $bad -eq 1 ]; then
		echo "gmt_launch_jobs: Your number of commands is not a multiple of $n_lines" >&2
		exit 1
	fi
	if [ $n_chunks -lt $n_cpu ]; then
		echo "gmt_launch_jobs: Less chunks than cores; only using $n_cpu processors" >&2
		let n_cpu=n_chunks
	fi
	if [ $dryrun -eq 1 ]; then
		tag=
	else
		tag="."$$
	fi
	# Create n_cpu empty files for execution
	let cpu=0
	while [ $cpu -lt $n_cpu ]; do
		printf "#!/bin/bash\n# gmt_launch_jobs command file chunk # ${cpu}\n#---------------------------------\n" > /tmp/gmt_launch_jobs${tag}.$cpu.sh
		let cpu=cpu+1
	done
	# Distribute $n_lines from the commands across these core scripts
	let chunk=0; let cpu=0; let sub=n_lines-1; let last=0
	while [ $chunk -lt $n_chunks ]; do
		let last=last+n_lines
		let first=last-sub
		sed -n ${first},${last}p /tmp/$$.sh >> /tmp/gmt_launch_jobs${tag}.$cpu.sh
		let cpu=cpu+1
		if [ $cpu -eq $n_cpu ]; then
			let cpu=0
		fi
		let chunk=chunk+1
	done
	# Launch the $n_cpu scripts
	let cpu=0
	while [ $cpu -lt $n_cpu ]; do
		if [ $blabber -eq 1 ]; then
			echo "gmt_launch_jobs: Starting /tmp/gmt_launch_jobs${tag}.$cpu.sh" >&2
		fi
		if [ $do_remove -eq 1 ]; then
			echo "rm -f /tmp/gmt_launch_jobs${tag}.$cpu.sh" >> /tmp/gmt_launch_jobs${tag}.$cpu.sh
		fi
		if [ $dryrun -eq 0 ]; then
			bash /tmp/gmt_launch_jobs${tag}.$cpu.sh &
		fi
		let cpu=cpu+1
	done
	rm -f /tmp/$$.sh
	if [ $do_wait -eq 1 ] && [ $dryrun -eq 0 ]; then
		wait	# Wait until all jobs lauched by this script completes
		if [ $blabber -eq 1 ]; then
			echo "gmt_launch_jobs: All $n_cpu jobs completed" >&2
		fi
	fi
}
#===================================================================================
# OBSOLETE: These functions are supported but obsoleted by new GMT built-in features
#===================================================================================

# Backwards compatible functions in use before -W was introduced in mapproject 5.2:
#	Return the current map width (expects -R and -J settings)
gmt_get_map_width() {
	gmt mapproject $* -Ww
}
#	Same with backwards compatible name...
gmt_map_width() {
	gmt mapproject $* -Ww
}

#	Return the current map height (expects -R and -J settings)
gmt_get_map_height() {
	gmt mapproject $* -Wh
}
#	and with backwards compatible name...
gmt_map_height() {
	gmt mapproject $* -Wh
}
#	Return integer total number of lines in the file(s). Obsolete since 5.4.0
gmt_get_nrecords() {
	gmt info -Fi -o4 $*
}
#	Same with backwards compatible name...
gmt_nrecords() {
	gmt info -Fi -o4 $*
}

#	Return integer total number of data records in the file(s). Obsolete since 5.4.0
gmt_get_ndatarecords() {
	gmt info -Fi -o2 $*
}
