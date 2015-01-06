# $Id$
#
# Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo,
# J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# If you source this file from your .bashrc, bash should be able to
# tab-complete a command line that uses gmt with all the available
# commands and options.

# check for bash
[ -z "$BASH_VERSION" ] && return

# provide _filedir if not already defined
if ! type -t _filedir >/dev/null 2>&1 ; then
    _gmt_comp_option="-o filenames"
    _filedir()
    {
        local IFS=$'\n'
        COMPREPLY=( ${COMPREPLY[@]} $( compgen -o plusdirs -f -- "$cur" ) )
    }
fi

_gmt_in_array()
{
    local hay needle=$1
    shift
    for hay; do
        [[ $hay == $needle ]] && return 0
    done
    return 1
}

_gmt()
{
    local arg1 cur fileopts modopts opts progs
    COMPREPLY=()
    arg1="${COMP_WORDS[1]}"
    cur="${COMP_WORDS[COMP_CWORD]}"
    fileopts=()

    opts=( --help --show-datadir --show-bindir --version )

    progs=( 2kml backtracker blockmean blockmedian blockmode connect convert \
        defaults dimfilter filter1d fitcircle flexure get gmt2kml gmtconnect gmtconvert \
        gmtdefaults gmtflexure gmtget gmtgravmag3d gmtinfo gmtlogo gmtmath gmtselect gmtset \
        gmtsimplify gmtspatial gmtvector gmtwhich gravfft grd2cpt grd2rgb \
        grd2xyz grdblend grdclip grdcontour grdcut grdedit grdfft grdfilter grdflexure \
        grdgradient grdgravmag3d grdhisteq grdimage grdinfo grdlandmask grdmask \
        grdmath grdpaste grdpmodeler grdproject grdraster grdredpol grdconvert \
        grdrotater grdsample grdseamount grdspotter grdtrack grdtrend grdvector \
        grdview grdvolume greenspline gshhg hotspotter img2grd info kml2gmt logo \
        makecpt mapproject math mgd77convert mgd77info mgd77list mgd77magref \
        mgd77manage mgd77path mgd77sniffer mgd77track nearneighbor originator \
        project psconvert psbasemap psclip pscoast pscontour pscoupe pshistogram \
        psimage pslegend psmask psmeca pspolar psrose psscale pssegy pssegyz \
        pstext psvelo pswiggle psxy psxyz rotconverter sample1d segy2grd select \
        set simplify spatial spectrum1d sph2grd sphdistance sphinterpolate \
        sphtriangulate splitxyz surface trend1d trend2d triangulate vector which \
        x2sys_binlist x2sys_cross x2sys_datalist x2sys_get x2sys_init x2sys_list \
        x2sys_merge x2sys_put x2sys_report x2sys_solve xyz2grd )

    # complete first arg
    if [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=($(compgen -W '${opts[@]} ${progs[@]}' -- ${cur}))
        return 0
    fi

    # if first arg is ins progs
    if [[ "${progs[@]}" =~ "${arg1}" ]] && _gmt_in_array "${arg1}" "${progs[@]}"; then
        case ${arg1} in
            backtracker)
                modopts=( -: -A -D -E -F -L -N -Q -S -T -V -W -b -f -g -h -i -o -s )
                fileopts=( -E )
                ;;
            blockmean)
                modopts=( -: -C -E -I -R -S -V -W -a -b -f -g -h -i -o -r )
                ;;
            blockmedian)
                modopts=( -: -C -E -I -Q -R -T -V -W -a -b -f -g -h -i -o -r )
                ;;
            blockmode)
                modopts=( -: -C -D -E -I -Q -R -V -W -a -b -f -g -h -i -o -r )
                ;;
            dimfilter)
                modopts=( -: -D -F -G -I -N -Q -R -T -V -f -h )
                fileopts=( -G )
                ;;
            filter1d)
                modopts=( -: -D -E -F -I -L -N -Q -S -T -V -a -b -f -g -h -i -o )
                ;;
            fitcircle)
                modopts=( -: -L -S -V -a -b -f -g -h -i -o )
                ;;
            gmt2kml|2kml)
                modopts=( -: -A -C -D -E -F -G -I -K -L -N -O -Q -R -S -T -V -W -Z -a -b -f -g -h -i )
                fileopts=( -C -D )
                ;;
            gmtconnect|connect)
                modopts=( -: -C -D -L -Q -T -V -a -b -f -g -h -i -o -s )
                fileopts=( -L )
                ;;
            gmtconvert|convert)
                modopts=( -: -A -D -E -I -L -Q -S -T -V -a -b -f -g -h -i -o -s )
                ;;
            gmtdefaults|defaults)
                modopts=( -D -V )
                ;;
            gmtflexure|flexure)
                modopts=( -A -C -D -F -S -T -V -W -Z -h )
                ;;
            gmtget|get)
                modopts=( -G -L -V \
                    $(gmt defaults | awk '/^[^#]/ {printf "%s ", $1}') )
                fileopts=( -G )
                ;;
            gmtgravmag3d|gravmag3d)
                modopts=( -: -C -E -F -G -H -I -L -R -S -T -V -Z -f -r )
                fileopts=( -F -G -T )
                ;;
            gmtinfo|info)
                modopts=( -: -A -C -D -E -I -S -T -V -b -f -g -h -i -o -r -s )
                ;;
            gmtlogo|logo)
                modopts=( -D -F -K -O -P -W -V -X -Y -c -t -x -y )
                ;;
            gmtmath|math)
                modopts=( -: -A -C -E -I -L -N -Q -S -T -V -b -f -g -h -i -o -s \
                    $(gmt gmtmath 2>&1 | awk '/^\t[A-Z0-9]*[\t ]*[ ][1-3=] / {printf "%s ", $1}') )
                fileopts=( -T )
                ;;
            gmtselect|select)
                modopts=( -: -A -C -D -E -F -I -J -L -N -R -V -Z -a -b -f -g -h -i -o -s )
                fileopts=( -C -F -L )
                ;;
            gmtset|set)
                modopts=( -B -C -D -G -J -R -V -X -Y -c -p \
                    $(gmt defaults | awk '/^[^#]/ {printf "%s= ", $1}') )
                fileopts=( -G )
                ;;
            gmtsimplify|simplify)
                modopts=( -: -G -T -V -b -f -g -h -i -o )
                ;;
            gmtspatial|spatial)
                modopts=( -: -A -C -D -E -I -L -Q -R -S -T -V -a -b -f -g -h -i -o -s )
                fileopts=( -D -N )
                ;;
            gmtvector|vector)
                modopts=( -: -A -C -E -N -S -T -V -b -f -g -h -i -o -s )
                ;;
            gmtwhich|which)
                modopts=( -A -C -D -V )
                ;;
            gravfft)
                modopts=( -C -D -E -F -G -I -N -Q -T -V -Z -f )
                fileopts=( -G )
                ;;
            grd2cpt)
                modopts=( -A -C -D -E -F -G -I -L -M -N -Q -R -S -T -V -Z -h )
                fileopts=( -C )
                ;;
            grd2rgb)
                modopts=( -C -G -I -L -R -V -W -h -r )
                fileopts=( -C )
                ;;
            grd2xyz)
                modopts=( -: -C -N -R -V -W -Z -b -f -h -o -s )
                ;;
            grdblend)
                modopts=( -: -C -G -I -N -Q -R -V -W -Z -f -r )
                fileopts=( -G )
                ;;
            grdclip)
                modopts=( -G -R -S -V )
                fileopts=( -G )
                ;;
            grdcontour)
                modopts=( -A -B -C -D -F -G -J -K -L -O -P -Q -R -S -T -U -V -W -X -Y -Z -b -c -f -h -p -t -x -y )
                fileopts=( -C )
                ;;
            grdcut)
                modopts=( -G -N -R -S -V -Z -f )
                fileopts=( -G )
                ;;
            grdedit)
                modopts=( -: -A -D -E -N -R -S -T -V -b -f -h -i )
                ;;
            grdfft)
                modopts=( -A -C -D -E -F -G -I -N -S -V -f -h )
                fileopts=( -G )
                ;;
            grdfilter)
                modopts=( -D -F -G -I -N -R -T -V -f )
                fileopts=( -G )
                ;;
            grdflexure)
                modopts=( -A -C -D -E -F -G -L -M -N -S -T -V -W -Z -f )
                ;;
            grdgradient)
                modopts=( -A -D -E -G -N -R -S -V -f )
                fileopts=( -G -S )
                ;;
            grdgravmag3d)
                modopts=( -: -C -D -F -G -I -L -Q -R -V -Z -f )
                fileopts=( -F -G )
                ;;
            grdhisteq)
                modopts=( -C -D -G -N -Q -R -V -h )
                fileopts=( -D -G )
                ;;
            grdimage)
                modopts=( -A -B -C -D -E -G -I -J -K -M -N -O -P -Q -R -T -U -V -X -Y -c -f -p -t -x -y )
                fileopts=( -A -C -G -I )
                ;;
            grdinfo)
                modopts=( -C -F -I -L -M -R -T -V -f -h )
                ;;
            grdlandmask)
                modopts=( -A -D -E -G -I -N -R -V -r )
                fileopts=( -G )
                ;;
            grdmask)
                modopts=( -: -A -G -I -N -R -S -V -a -b -f -g -h -i -r -s )
                fileopts=( -G )
                ;;
            grdmath)
                modopts=( -: -I -M -N -R -V -b -f -g -h -i -r -s \
                    $(gmt grdmath 2>&1 | awk '/^\t[A-Z0-9]*[\t ]*[ ][1-3=] / {printf "%s ", $1}') )
                ;;
            grdpaste)
                modopts=( -G -V -f )
                fileopts=( -G )
                ;;
            grdpmodeler)
                modopts=( -: -E -F -G -I -R -S -T -V -b -h -i -r )
                fileopts=( -E -F -G )
                ;;
            grdproject)
                modopts=( -A -C -D -E -G -I -J -M -R -V -r )
                fileopts=( -G )
                ;;
            grdraster)
                modopts=( -G -I -J -R -T -V -b -h -o )
                fileopts=( -G )
                ;;
            grdredpol)
                modopts=( -C -E -F -G -M -N -R -T -V -W -Z )
                fileopts=( -E -G -Z )
                ;;
            grdconvert)
                modopts=( -N -R -V -f )
                ;;
            grdrotater)
                modopts=( -: -D -E -F -G -N -R -S -T -V -b -f -g -h -i -o )
                fileopts=( -E -F -G )
                ;;
            grdsample)
                modopts=( -G -I -R -T -V -f -r )
                fileopts=( -G )
                ;;
            grdseamount)
                modopts=( -: -A -C -E -G -I -L -N -R -S -T -V -Z -b -f -h -i -r )
                fileopts=( -G )
                ;;
            grdspotter)
                modopts=( -: -A -D -E -G -I -L -M -N -Q -R -S -T -V -W -Z -h -r -u )
                fileopts=( -A -D -E -L -P  )
                ;;
            grdtrack)
                modopts=( -: -A -C -D -E -G -N -R -S -T -V -Z -a -b -f -g -h -i -o -s )
                fileopts=( -D -G )
                ;;
            grdtrend)
                modopts=( -D -N -R -T -V -W )
                ;;
            grdvector)
                modopts=( -A -B -C -G -I -J -K -N -O -P -Q -R -S -T -U -V -W -X -Y -Z -c -f -p -t -x -y )
                fileopts=( -C )
                ;;
            grdview)
                modopts=( -B -C -G -I -J -K -N -O -P -Q -R -S -T -U -V -W -X -Y -c -f -p -t -x -y )
                fileopts=( -C -G -I -N )
                ;;
            grdvolume)
                modopts=( -C -L -R -S -T -V -Z -f -h -o )
                ;;
            greenspline)
                modopts=( -: -A -C -D -G -I -L -N -Q -R -S -T -V -W -b -f -g -h -i -o -r -s )
                fileopts=( -A -C -G -N -T )
                ;;
            gshhg)
                modopts=( -: -A -G -I -L -N -Q -V -b -o )
                ;;
            hotspotter)
                modopts=( -: -D -E -G -I -N -R -S -T -V -b -g -h -i -r -s )
                fileopts=( -E -G )
                ;;
            img2grd)
                modopts=( -C -D -E -G -I -M -N -R -S -T -V -W )
                fileopts=( -G )
                ;;
            kml2gmt)
                modopts=( -: -V -Z -b -h )
                ;;
            makecpt)
                modopts=( -A -C -D -F -G -I -M -N -Q -T -V -Z -h )
                ;;
            mapproject)
                modopts=( -: -A -C -D -E -F -G -I -J -L -N -Q -R -S -T -V -b -f -g -h -i -o -s )
                ;;
            mgd77convert)
                modopts=( -C -D -F -L -T -V )
                ;;
            mgd77info)
                modopts=( -C -E -I -L -M -V )
                ;;
            mgd77list)
                modopts=( -: -A -C -D -E -F -G -I -L -N -Q -R -S -T -V -W -Z -b -h )
                ;;
            mgd77magref)
                modopts=( -: -A -C -D -E -F -G -L -S -V -b -h )
                fileopts=( -C -D -E )
                ;;
            mgd77manage)
                modopts=( -A -C -D -E -F -I -N -R -V -b )
                ;;
            mgd77path)
                modopts=( -D -I -V )
                ;;
            mgd77sniffer)
                modopts=( -A -C -D -G -H -I -K -L -N -R -S -T -V -W -b )
                fileopts=( -G -L )
                ;;
            mgd77track)
                modopts=( -A -B -C -D -F -G -I -J -K -L -N -O -P -R -S -T -U -V -W -X -Y -c -p -t -x -y )
                ;;
            nearneighbor)
                modopts=( -: -E -G -I -N -R -S -V -W -b -f -h -i -r -s )
                fileopts=( -G )
                ;;
            originator)
                modopts=( -: -D -E -F -H -L -N -Q -S -T -V -W -Z -b -h -i -s )
                fileopts=( -E -F  )
                ;;
            project)
                modopts=( -: -A -C -E -F -G -L -N -Q -S -T -V -W -b -f -g -h -i -s )
                ;;
            psconvert)
                modopts=( -A -C -D -E -F -G -L -N -P -Q -S -TE -TF -TG -Tb -Te -Tf -Tg -Tj -Tm -Tt -V -W )
                fileopts=( -D -G -L )
                ;;
            psbasemap)
                modopts=( -B -D -J -K -L -O -P -R -T -U -V -X -Y -c -f -p -t -x -y )
                ;;
            psclip)
                modopts=( -: -A -B -C -J -K -N -O -P -R -T -U -V -X -Y -b -c -f -g -h -i -p -s -t -x -y )
                ;;
            pscoast)
                modopts=( -: -A -B -C -D -F -G -I -J -K -L -M -N -O -P -Q -R -S -T -U -V -W -X -Y -b -c -p -t -x -y )
                ;;
            pscontour)
                modopts=( -: -A -B -C -D -G -I -J -K -L -N -O -P -Q -R -S -T -U -V -W -X -Y -b -c -h -i -p -s -t -x -y )
                fileopts=( -C -Q )
                ;;
            pscoupe)
                modopts=( -: -A -B -E -F -G -H -J -K -L -M -N -O -P -R -S -T -U -V -W -X -Y -Z -c -h -i -x -y )
                fileopts=( -Z )
                ;;
            pshistogram)
                modopts=( -5 -A -B -C -D -F -G -I -J -K -L -N -O -P -Q -R -S -U -V -W -X -Y -Z -b -c -f -h -i -p -s -t -x -y )
                fileopts=( -C )
                ;;
            psimage)
                modopts=( -C -E -F -G -I -J -K -M -N -O -P -R -U -V -W -X -Y -c -p -t -x -y )
                ;;
            pslegend)
                modopts=( -B -C -D -F -J -K -L -O -P -R -U -V -X -Y -c -p -t )
                ;;
            psmask)
                modopts=( -: -B -C -D -G -I -J -K -N -O -P -Q -R -S -T -U -V -X -Y -b -c -h -i -p -r -s -t -x -y )
                fileopts=( -D )
                ;;
            psmeca)
                modopts=( -: -B -C -D -E -F -G -H -J -K -L -M -N -O -P -R -S -T -U -V -W -X -Y -Z -c -h -i -x -y -z )
                fileopts=( -Z )
                ;;
            pspolar)
                modopts=( -: -A -B -C -D -E -F -G -H -J -K -M -N -O -P -Q -R -S -T -U -V -W -X -Y -c -h -i -s -x -y )
                ;;
            psrose)
                modopts=( -: -A -B -C -D -G -I -K -L -M -N -O -P -R -S -T -U -V -W -X -Y -Z -b -c -h -i -p -s -t -x -y )
                fileopts=( -C )
                ;;
            psscale)
                modopts=( -A -B -C -D -E -G -I -J -K -L -M -N -O -P -Q -R -S -T -U -V -X -Y -Z -c -p -t -x -y )
                fileopts=( -C -Z )
                ;;
            pssegy)
                modopts=( -A -B -C -D -E -F -I -J -K -L -M -N -O -P -Q -R -S -T -U -V -W -X -Y -Z -c -p -t )
                ;;
            pssegyz)
                modopts=( -A -B -C -D -E -F -I -J -K -L -M -N -O -P -Q -R -S -T -U -V -W -X -Y -Z -c -p -t )
                fileopts=( -T )
                ;;
            pstext)
                modopts=( -: -A -B -C -D -F -G -J -K -L -M -N -O -P -Q -R -T -U -V -W -X -Y -Z -a -c -f -h -p -t -x -y )
                ;;
            psvelo)
                modopts=( -: -A -B -G -H -J -K -L -N -O -P -R -S -U -V -W -X -Y -c -h -i -x -y )
                ;;
            pswiggle)
                modopts=( -: -A -B -C -G -I -J -K -O -P -R -S -T -U -V -W -X -Y -Z -b -c -f -g -h -i -p -s -t -x -y )
                ;;
            psxy)
                modopts=( -: -A -B -C -D -E -G -I -J -K -L -N -O -P -R -S -T -U -V -W -X -Y -a -b -c -f -g -h -i -p -s -t )
                fileopts=( -C )
                ;;
            psxyz)
                modopts=( -: -B -C -D -G -I -J -K -L -N -O -P -Q -R -S -U -V -W -X -Y -a -b -c -f -g -h -i -p -s -t -x -y )
                fileopts=( -C )
                ;;
            rotconverter)
                modopts=( -: -A -D -E -F -G -N -S -T -V -W -h )
                ;;
            sample1d)
                modopts=( -A -F -I -N -S -T -V -b -f -g -h -i -o -s )
                fileopts=( -N )
                ;;
            segy2grd)
                modopts=( -A -D -G -I -L -M -N -Q -R -S -V -r )
                fileopts=( -G -S )
                ;;
            spectrum1d)
                modopts=( -C -D -L -N -S -V -W -b -f -g -h -i -s )
                ;;
            sph2grd)
                modopts=( -D -E -F -G -I -N -Q -R -V -b -h -i -r -s )
                fileopts=( -G )
                ;;
            sphdistance)
                modopts=( -: -C -E -G -I -L -N -Q -R -V -b -h -i -r -s )
                fileopts=( -G )
                ;;
            sphinterpolate)
                modopts=( -: -G -I -Q -R -T -V -Z -b -h -i -r -s )
                fileopts=( -G )
                ;;
            sphtriangulate)
                modopts=( -: -A -C -D -L -N -Q -R -T -V -b -h -i -s )
                fileopts=( -N )
                ;;
            splitxyz)
                modopts=( -: -A -C -D -F -N -Q -S -V -Z -b -f -g -h -i -s )
                ;;
            surface)
                modopts=( -: -A -C -D -G -I -L -N -Q -R -S -T -V -Z -a -b -f -h -i -r -s )
                fileopts=( -G )
                ;;
            trend1d)
                modopts=( -: -C -F -I -N -V -W -b -f -h -i -s )
                fileopts=( -N )
                ;;
            trend2d)
                modopts=( -: -C -F -I -N -V -W -b -f -h -i -s )
                ;;
            triangulate)
                modopts=( -: -D -E -G -I -J -M -N -Q -R -S -V -Z -b -f -h -i -r -s )
                fileopts=( -G )
                ;;
            x2sys_binlist)
                modopts=( -D -E -T -V )
                ;;
            x2sys_cross)
                modopts=( -A -C -I -J -Q -R -S -T -V -W -Z -b )
                ;;
            x2sys_datalist)
                modopts=( -A -E -F -I -L -R -S -T -V -b )
                ;;
            x2sys_get)
                modopts=( -C -D -F -G -L -N -R -T -V )
                ;;
            x2sys_init)
                modopts=( -C -D -E -F -G -I -N -R -V -W -m )
                fileopts=( -D )
                ;;
            x2sys_list)
                modopts=( -A -C -E -F -I -L -N -Q -R -S -T -V -W -m )
                ;;
            x2sys_merge)
                modopts=( -A -M -V )
                ;;
            x2sys_put)
                modopts=( -D -F -T -V )
                ;;
            x2sys_report)
                modopts=( -A -C -I -L -N -Q -R -S -T -V )
                ;;
            x2sys_solve)
                modopts=( -C -E -T -V -W -b )
                ;;
            xyz2grd)
                modopts=( -: -A -D -G -I -N -R -S -V -Z -b -f -h -i -r -s )
                fileopts=( -G -S )
                ;;
            *)
                modopts=( )
                ;;
        esac

        # expand files and directories after options contained in ${fileopts}
        fileopts=( "${fileopts[@]}" -R )
        if [[ "${fileopts[@]}" =~ "${cur:0:2}" ]] && _gmt_in_array "${cur:0:2}" "${fileopts[@]}"; then
            local cnt opt
            opt=${cur:0:2}
            cur="${cur:2}"
            _filedir
            cnt=${#COMPREPLY[@]}
            for ((i=0;i<cnt;i++)); do
                COMPREPLY[i]="${opt}${COMPREPLY[i]}"
            done
            return 0
        fi

        COMPREPLY=($(compgen -W '-? -^ ${modopts[@]}' -- ${cur}))
        _filedir

    fi
} &&
complete ${_gmt_comp_option} -o nospace -F _gmt gmt

# ex: ts=4 sw=4 et filetype=sh
