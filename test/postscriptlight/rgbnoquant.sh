#!/usr/bin/env bash
#
# Purpose: Verify RGBnoQUANT bypasses PSL indexed-color image optimization
# GMT modules: gmt
#
printf 'P6\n4 4\n255\n' > colors.ppm
printf '\377\000\000\000\377\000\000\000\377\377\377\000' >> colors.ppm
printf '\000\377\000\000\000\377\377\377\000\377\000\000' >> colors.ppm
printf '\000\000\377\377\377\000\377\000\000\000\377\000' >> colors.ppm
printf '\377\377\000\377\000\000\000\377\000\000\000\377' >> colors.ppm

# Default RGB keeps the existing indexed-color and bit-reduction path.
gmt set PS_COLOR_MODEL RGB
[ "$(gmt get PS_COLOR_MODEL)" = "rgb" ] || echo "RGB setting did not round-trip" >> fail
gmt psimage colors.ppm -R0/4/0/4 -JX4c > indexed.ps
grep -Fq "/Indexed /DeviceRGB" indexed.ps || echo "RGB image was not indexed" >> fail
grep -Fq "/Decode [0 3]" indexed.ps || echo "RGB image was not bit-reduced" >> fail

# Case-insensitive RGBnoQUANT must use the existing direct DeviceRGB path.
gmt set PS_COLOR_MODEL RgBnOqUaNt
[ "$(gmt get PS_COLOR_MODEL)" = "rgbnoquant" ] || echo "RGBnoQUANT setting did not round-trip" >> fail
gmt psimage colors.ppm -R0/4/0/4 -JX4c > direct.ps
grep -Fq "/Indexed /DeviceRGB" direct.ps && echo "RGBnoQUANT image was indexed" >> fail
grep -Fq "/DeviceRGB setcolorspace" direct.ps || echo "RGBnoQUANT image was not direct DeviceRGB" >> fail
grep -Fq "/Decode [0 1 0 1 0 1]" direct.ps || echo "RGBnoQUANT image was not 24 bit" >> fail

# Switching back to RGB must reset the no-quantization flag.
gmt set PS_COLOR_MODEL rgb
gmt psimage colors.ppm -R0/4/0/4 -JX4c > restored.ps
grep -Fq "/Indexed /DeviceRGB" restored.ps || echo "RGB did not restore indexed output" >> fail
