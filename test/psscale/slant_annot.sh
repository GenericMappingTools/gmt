#!/usr/bin/env bash
# Test implementation of -S+a<angle> for slanted annotations
# when -B is not used.
gmt begin slant_annot ps
	gmt makecpt -Cbatlow -T5000/20000/1000
	gmt colorbar -C -Dx0/23c+jBL+w15c/0.4c+h+m
	gmt colorbar -C -Dx0/22c+jBL+w15c/0.4c+h
	gmt colorbar -C -Dx0/20.5c+jBL+w15c/0.4c+h -S+a45
	gmt colorbar -C -Dx0/18.5c+jBL+w15c/0.4c+h -S+a-45
	gmt colorbar -C -Dx0/15.5c+jBL+w15c/0.4c+h+m -S+a45
	gmt colorbar -C -Dx0/13.5c+jBL+w15c/0.4c+h+m -S+a-45
	gmt colorbar -C -Dx0/0c+jBL+w12c/0.4c
	gmt colorbar -C -Dx4c/0c+jBL+w12c/0.4c+m
	gmt colorbar -C -Dx5.5c/0c+jBL+w12c/0.4c -S+a45
	gmt colorbar -C -Dx8c/0c+jBL+w12c/0.4c -S+a-45
	gmt colorbar -C -Dx12c/0c+jBL+w12c/0.4c+m -S+a45
	gmt colorbar -C -Dx14.5c/0c+jBL+w12c/0.4c+m -S+a-45
gmt end show
