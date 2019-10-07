#!/usr/bin/env bash
gmt set FORMAT_DATE_MAP "o yy" FORMAT_TIME_PRIMARY_MAP Abbreviated
gmt basemap -R1996T/1996-6T/0/1 -JX5i/0.2i -Ba1Of1d -BS -ps GMT_-B_time6
