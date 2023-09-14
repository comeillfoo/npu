#!/usr/bin/env python3
from PIL import Image, ImageTk
import numpy as np

def bin2image(file: str, width: int, height: int) -> Image:
    vector = None
    with open(file, 'rb') as f:
        vector = f.read()
    if len(vector) != width * height:
        raise AttributeError
    vector = np.reshape(list(map(bool, vector)), (width, height))
    img = Image.fromarray(vector)
    img = img.resize((width * 20, height * 20), Image.Resampling.BICUBIC)
    return img # one pixel per byte


def bin2image_tk(file: str, width: int, height: int) -> ImageTk:
    return ImageTk.PhotoImage(bin2image(file, width, height)) # one pixel per byte
