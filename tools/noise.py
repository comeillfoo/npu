#!/usr/bin/python3
import argparse, random as r


def parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser('noise',
                                description='Noise the bit')
    p.add_argument('input')
    p.add_argument('-o', '--out', default='m.out', help='path to output')
    p.add_argument('-i', '--invert', type=int,
                   default=1, help='number of invert bits')
    p.add_argument('-w', '--width', default=7, help='width of image')
    p.add_argument('--height', default=7, help='height of image')
    p.add_argument('-s', '--seed', type=int, help='seed for random generators')
    return p


def main() -> int:
    args = parser().parse_args()
    # TODO: test input file existance

    vector = [] # vectorized grayscale image
    with open(args.input, 'rb') as input:
        vector = input.read()

    for i in range(args.invert):
        # TODO: handle already inverted pixel
        (x, y) = (r.randrange(args.width), r.randrange(args.height))
    return 0


if __name__ == '__main__':
    exit(main())
