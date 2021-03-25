#!/usr/bin/env bash

#	WIP: Explain
	
#	Titulo del mapa
	title=anim14
	echo $title
	
#	Proyeccion del mapa y ancho del mapa
	W=23.64c
	W=22.3c

#	Dimensiones del Grafico (en cm): Ancho (L), Altura inferior (H1) y arriba (H2)
	H=3.5
	H1=3.8
	PROJ=M$W
	
	DEM="/home/federico/E:/Facultad/Datos_Geofisicos/Batimetria/SRTM15+v21/SRTM15+V2.1.nc"	
#	DEM=@earth_relief_15s
	
#	Region geografica del mapa (W/E/S/N)
	REGION=-75/-61/-33.2/-28.7 # v0

#	Coordendas iniciales (1) y finales del perfil (2)
	Long1=-74
	Long2=-64
	Lat1=-29
	Lat2=-33
	KM=1052.32

#	Distancia perpendicular al pefil (en km) y rango de profundidades del perfil (en km)
	Dist_Perfil=100
	DepthMin=0
	DepthMax=200

#	Offset en X/Y
	X=0.32
	Y=0.91
	
#       -----------------------------------------------------------------------------------------------------------
cat << EOF > pre.sh

	gmt set COLOR_HSV_MIN_V 0
	gmt set FONT_LABEL 10p
	gmt set FONT_ANNOT_PRIMARY 7p
	gmt set MAP_FRAME_PEN thin,black
	
gmt begin

#	Crear lista de fechas para la animacion: Inicio/Fin/Intervalo. o: meses. y: años
	gmt math -o0 -T2005-01-01T/2020-12-1T/10y T = times.txt
	gmt math -o0 -T2005-01-01T/2020-12-1T/1o T = times.txt
#	gmt math -o0 -T2005-01-01T/2020-12-1T/7d T = times.txt

	gmt makecpt -T0/$DepthMax -Cbatlow -I -H > q.cpt

#       -----------------------------------------------------------------------------------------------------------
#	Eje X (Sn) e Y
	gmt basemap -R0/$KM/$DepthMin/$DepthMax -JX$W/-$H -Bxaf+l"Distance (km)" -Byaf+l"Depth (km)" -Y$Y -X$X  --MAP_FRAME_AXES=wESn 

	gmt grdimage $DEM -Coleron -I+nt1.2 -R$REGION -J$PROJ -Y$H1  # Map

#	Dibujar Bordes Administrativos. N1: paises. N2: Provincias, Estados, etc. N3: limites marítimos (Nborder[/pen])
	gmt coast -Df -N1/0.30 -N2/0.2,-

#	gmt basemap -Bxf1 -Byf1
	gmt basemap -Baf --MAP_FRAME_AXES=wEsN
	gmt basemap -Ln0.87/0.1+w100k+f+u -F+p+gwhite+s+r

	gmt plot -W1p,red <<- END
	$Long1 $Lat1
	$Long2 $Lat2
	END
		
gmt end
EOF

#	----------------------------------------------------------------------------------------------------------
# 	2. Set up main script
cat << EOF > main.sh
gmt begin
	gmt set COLOR_HSV_MIN_V 0
	gmt set FONT_LABEL 6p
	gmt set FONT_ANNOT_PRIMARY 6p

	gmt events -R0/$KM/$DepthMin/$DepthMax -JX$W/-$H -Y$Y -X$X "Datos_Mec.txt" -Z"coupe -Aa$Long1/$Lat1/$Long2/$Lat2/90/100/$DepthMin/$DepthMax -Q -Sd0.3c+f0" -Wfaint -T\${MOVIE_COL0} -Es+r2+p1+d6+f1 -Mi1+c-0.5 -Ms1.5+c0.8 -Mt+c10 --TIME_UNIT=o  -Cq.cpt 
	gmt events -R$REGION -J$PROJ "Datos_Mec.txt" -Z"meca -Sd0.3c+f0" -Wfaint -T\${MOVIE_COL0} -Es+r2+p1+d6+f1 -Mi1+c-0.5 -Ms1.5+c0.8 -Mt+c10 --TIME_UNIT=o -Y$H1 -Cq.cpt 
gmt end
EOF

#	----------------------------------------------------------------------------------------------------------
# 	3. Run the movie
gmt movie main.sh -Sbpre.sh -Chd -Ttimes.txt -N$title -H2 -Lc0+jTR+o1.7/0.6+gwhite+h+r --FONT_TAG=13p,Helvetica,black --FORMAT_CLOCK_MAP=- --FORMAT_DATE_MAP=o-yyyy -Fmp4 -D14 -Ml,png -Ve -Zs -Pd+ap+jRM+w6.2c+o1.8/1.8c
