#!/usr/bin/env bash
# Make sure lakes are not set if only river-lakes are filled.
# Script used in showing the bug from https://github.com/GenericMappingTools/gmt/issues/6038
# The red circle should be visible as lake should not be filled
gmt begin riverlake
    echo "50 42" | gmt plot -JQ50/42/10c -R46/54/36/48 -Sc1c -W1p,black -Gred
    gmt coast -Dl -Ggray -Cgray+r
gmt end show
