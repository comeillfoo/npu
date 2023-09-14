#!/usr/bin/env python3
from tkinter import *
import argparse
from bin2img import bin2image

# parse arguments
p = argparse.ArgumentParser('binshow')
p.add_argument('input', help='path to image in binary')
p.add_argument('-w', '--width', default=7, help='image width')
p.add_argument('--height', default=7, help='image height')
args = p.parse_args()
img = bin2image(args.input, args.width, args.height)
img.show()
