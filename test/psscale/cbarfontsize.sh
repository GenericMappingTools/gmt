#!/usr/bin/env bash
# Test implementation of --PARAM=value with themes

gmt begin cbarfontsize ps
	gmt makecpt -Cbatlow -T0/20/4
	gmt colorbar -C -Dx0/12c+jBL+w15c/0.4c+h
	gmt set FONT_ANNOT_PRIMARY 20p
	gmt colorbar -C -Dx0/8c+jBL+w15c/0.4c+h
	gmt colorbar -C -Dx0/4c+jBL+w15c/0.4c+h --FONT_ANNOT_PRIMARY=32p
gmt end show
