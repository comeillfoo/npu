#!/usr/bin/env python3
from tkinter import *
import os, argparse, shutil
from bin2img import bin2image_tk


# parse arguments
p = argparse.ArgumentParser('bin2class')
p.add_argument('data', help='folder with dataset')
p.add_argument('-w', '--width', default=7, help='image width')
p.add_argument('--height', default=7, help='image height')
p.add_argument('-c', '--classes', default=['circles', 'squares', 'triangles'], nargs='*')
args = p.parse_args()

# configure app
root = Tk()

fullpath = lambda name: os.path.join(args.data, name)
def converter(file: str) -> tuple:
    return (file, bin2image_tk(file, args.width, args.height))


# prepare dataset
dataset = { file: image
           for file, image in map(converter,
                                  filter(lambda path: os.path.isfile(path),
                                         map(fullpath, os.listdir(args.data)))) }
files = set(dataset.keys())

ius = Label(root, image=None)
ius.pack()

filevar = StringVar(root, value=None)
filename = Label(root, textvariable=filevar)
filename.pack()


def next_image(filevar: StringVar):
    if not files:
        root.destroy()
        return
    if filevar.get() != '':
        print(f'processed {filevar.get()}, remaining: {len(files) - 1}')
    file = files.pop()
    filevar.set(file)
    ius.config(image=dataset[file])


# control buttons
# create nested folders for classes
# print(args.classes)
for class_ in args.classes:
    class_dir = os.path.join(args.data, class_)
    os.mkdir(class_dir)
    def class_btn_handler_generator(class_dir: str):
        def class_btn_handler():
            shutil.copy(filevar.get(), class_dir)
            next_image(filevar)
        return class_btn_handler

    class_btn = Button(root, text=class_, command=class_btn_handler_generator(class_dir))
    class_btn.pack()

def discard_btn_handler():
    # delete file
    os.remove(filevar.get())
    next_image(filevar)

discard_btn = Button(root, text='discard', command=discard_btn_handler)
discard_btn.pack()

next_image(filevar)

root.mainloop()
