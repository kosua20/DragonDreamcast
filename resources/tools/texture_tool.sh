# Regular texture (paletted)
pvrtex -i $NAMETEX.png -o ../../data/$NAMETEX.dt -f pal8bpp -d -p preview-$NAMETEX.png --compress
# Skybox texture (non paletted)
pvrtex -i $NAMETEX.png -o ../../data/$NAMETEX.dt -f rgb565 -d -p preview-$NAMETEX.png --compress