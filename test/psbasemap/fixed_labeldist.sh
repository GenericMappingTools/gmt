#!/usr/bin/env bash
# Test a fixed label offset distance form all axes
# Script from https://github.com/GenericMappingTools/gmt/issues/5182
# The defaults MAP_LABEL_MODE = axis means the distance to the map label
# will be measured relative to the axis, not the top of the annotations.
# 
ps=fixed_labeldist.ps
gmt set FONT_LABEL 10p MAP_LABEL_OFFSET 8p/32p MAP_LABEL_MODE annot/axis FONT_ANNOT_PRIMARY 8p

plot_width=16c
plot_height=2.5c
xmin=6
xmax=17

############## C4

parameter="C4"
ymin=0
ymax=24.9
ytick=10
ysubtick=5
axis_label="C@-4@- (%)"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"
gmt psbasemap -BWSEn -Bxa2f1+l"Age (Ma)" -Bya${ytick}f${ysubtick}+l"${axis_label}" ${J_options} ${R_options} -P -K > $ps

############## MC total

parameter=MC_total
ymin=80
ymax=12000
axis_label="MC@-T@-"
J_options="-JX${plot_width}/${plot_height}l"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"
gmt psbasemap -Y${plot_height} -BWsEn -Bya1f3p+l"${axis_label} (grain/g)" ${J_options} ${R_options} -P -K -O >> $ps

############## MC_L / MC_R

parameter="Total_L_Total_R"
ymin=0.01
ymax=0.39
ytick=0.1
ysubtick=0.05
axis_label="MC@-L@-/MC@-R@-"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"
gmt psbasemap -Y${plot_height} -BWsEn -Bya${ytick}f${ysubtick}+l"${axis_label}" ${J_options} ${R_options} -P -O -K >> $ps

############## Grasses Yanwan

parameter="Yanwan_Grasses" 
ymin=0.1
ymax=99.9
ytick=20
ysubtick=10
axis_label="Grass (%)"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"
gmt psbasemap -Y${plot_height} -BWsEn -Bya${ytick}f${ysubtick}+l"${axis_label}" ${J_options} ${R_options} -P -O -K >> $ps

############## Grasses Yaodian

parameter="Yaodian_Grasses"
ymin=0.1
ymax=99.9
ytick=20
ysubtick=10
axis_label="Grass (%)"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"
gmt psbasemap -Y${plot_height} -BWsEn -Bya${ytick}f${ysubtick}+l"${axis_label}" ${J_options} ${R_options} -P -O -K >> $ps

############## Mammal fossils

ymin=0
ymax=2
y_half=1
ytick=1
ysubtick=5
axis_label="Fossils"
parameter="mammal_fossils"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"
gmt psbasemap -Y${plot_height} -BWsEn -Bxa100 -By+l"${axis_label}" ${J_options} ${R_options} -P -K -O >> $ps

############## Mammal fossils

ymin=2801
ymax=4099
y_half=1
ytick=1
ysubtick=5
axis_label="MSH"
parameter="MSH"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"
gmt psbasemap -Y${plot_height} -BWsEn -Bya400+200f100+l"${axis_label}" ${J_options} ${R_options} -P -K -O >> $ps

################## CO2

parameter="CO2"
ymin=50
ymax=399
ytick=100
ysubtick=25
axis_label="CO@-2@- (PPM)"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"

gmt psbasemap -Y${plot_height} -BWsEn -Bya${ytick}f${ysubtick}+l"${axis_label}" ${J_options} ${R_options} -P -O -K >> $ps

############ Tectonic

ymin=0
ymax=1
y_half=1
ytick=1
ysubtick=5
axis_label="Tectonics"
parameter="tectonic_events"
J_options="-JX${plot_width}/${plot_height}"
R_options="-R${xmin}/${xmax}/${ymin}/${ymax}"

gmt psbasemap -Y${plot_height} -BWsEn -Bxa100 -By+l"${axis_label}" ${J_options} ${R_options} -P -O >> $ps
