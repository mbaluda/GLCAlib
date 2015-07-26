#!/bin/sh
./GLblur img/in/baboon.rgba img/out/baboonblur.rgba 512 512
rm img/out/baboonblur.gif
convert -depth 8 -size 512x512 img/out/baboonblur.rgba img/out/baboonblur.gif