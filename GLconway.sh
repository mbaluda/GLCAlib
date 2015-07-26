#!/bin/sh
echo
echo ***GUI version***
./GLconway img/in/mem.rgba img/out/mem.rgba 512 512 0 300 1
echo
echo ***NOGUI version***
./GLconway img/in/mem.rgba img/out/mem.rgba 512 512 1 300 0
rm img/out/mem*gif
convert -depth 8 -size 512x512 img/out/mem.rgba img/out/mem.gif
convert -depth 8 -size 512x512 img/out/mem.rgbaCPU.rgba img/out/memCPU.gif