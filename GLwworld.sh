#!/bin/sh
echo
echo ***GUI version***
./GLwworld img/in/wworld.rgba img/out/wworld.rgba 800 600 0 300 1
echo
echo ***NOGUI version***
./GLwworld img/in/wworld.rgba img/out/wworld.rgba 800 600 1 300 0
rm img/out/wworld*gif
convert -depth 8 -size 800x600 img/out/wworld.rgba img/out/wworld.gif
convert -depth 8 -size 800x600 img/out/wworld.rgbaCPU.rgba img/out/wworldCPU.gif