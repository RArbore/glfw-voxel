#!/usr/bin/env python3
from PIL import Image
import sys

if len(sys.argv) < 3:
    print("Please provide a source image file and a destination title.")
    sys.exit(1)

image_fn = sys.argv[1];
title_fn = sys.argv[2];

try :
    pil_img = Image.open(image_fn).convert("RGB")
except FileNotFoundError:
    print("The source image file doesn't exist. Please provide a valid image path.")
    sys.exit(1)

fh = open("include/"+title_fn+".h", "w")
fc = open("source/"+title_fn+".c", "w")

w, h = pil_img.size

fh.write("#ifndef __TEXTURE_H_\n#define __TEXTURE_H_\n\nextern float texture["+str(w*h*3)+"];\n\n")
fc.write("float texture["+str(w*h*3)+"] = {")

for y in range(h):
    for x in range(w):
        r, g, b = pil_img.getpixel((w-x-1, y))
        r = float(r) / 255.0
        g = float(g) / 255.0
        b = float(b) / 255.0
        fc.write(str(r)+","+str(g)+","+str(b)+",")

fc.write("};")
fh.write("#define TEX_WIDTH "+str(w)+"\n")
fh.write("#define TEX_HEIGHT "+str(h)+"\n\n#endif // __TEXTURE_H_")
fh.close()
fc.close()
