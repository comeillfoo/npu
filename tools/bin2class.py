#!/usr/bin/python3
from tkinter import *
from PIL import ImageTk, Image
import os, argparse, numpy as np, shutil


def bin2image_tk(file: str, width: int, height: int) -> ImageTk:
    vector = None
    with open(file, 'rb') as f:
        vector = f.read()
    if len(vector) != width * height:
        raise AttributeError
    vector = np.reshape(list(map(bool, vector)), (width, height))
    img = Image.fromarray(vector)
    img = img.resize((width * 20, height * 20), Image.Resampling.BICUBIC)
    return ImageTk.PhotoImage(img) # one pixel per byte


# parse arguments
p = argparse.ArgumentParser('dataset_sorter')
p.add_argument('data', help='folder with dataset')
p.add_argument('-w', '--width', default=7, help='image width')
p.add_argument('--height', default=7, help='image height')
p.add_argument('-c', '--classes', default=['circles', 'squares', 'triangles'], nargs='*')
args = p.parse_args()


# configure app
root = Tk()

fullpath = lambda name: os.path.join(args.data, name)
def converter(name: str) -> tuple:
    file = fullpath(name)
    return (file, bin2image_tk(file, args.width, args.height))


# prepare dataset
dataset = { file: image
           for file, image in map(converter,
                                  filter(lambda path: not os.path.isfile(path), os.listdir(args.data))) }
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
            print('copy', filevar.get(), class_dir)
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