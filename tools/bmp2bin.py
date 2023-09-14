#!/usr/bin/env python3
from PIL import Image
import argparse, numpy as np


def parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser('bmp2bin')
    p.add_argument('image', help='input image')
    p.add_argument('-o', '--out', default='out.bin', help='path to output binary')
    return p


def main() -> int:
    args = parser().parse_args()
    pixels = np.array(Image.open(args.image)).reshape(-1)
    with open(args.out, 'wb') as out:
        out.write(bytearray(map(int, pixels)))
    return 0


if __name__ == '__main__':
    exit(main())
