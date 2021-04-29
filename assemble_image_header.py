#!/usr/bin/env python3
from PIL import Image
import sys

if len(sys.argv) < 3:
    print("Please provide a source image file and a destination header file.")
    sys.exit(1)

image_fn = sys.argv[1];
header_fn = sys.argv[2];

try :
    pil_img = Image.open(image_fn).convert("RGB")
except FileNotFoundError:
    print("The source image file doesn't exist. Please provide a valid image path.")
    sys.exit(1)

f = open(header_fn, "a")
f.write("float texture[] = {")

w, h = pil_img.size

for y in range(h):
    for x in range(w):
        r, g, b = pil_img.getpixel((w-x-1, y))
        r = float(r) / 255.0
        g = float(g) / 255.0
        b = float(b) / 255.0
        f.write(str(r)+","+str(g)+","+str(b)+",")

f.write("};\n")
f.write("int texture_width = "+str(w)+";\n")
f.write("int texture_height = "+str(h)+";\n")
f.close()
