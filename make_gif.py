import os

import imageio

img_path = './img_cache'

with imageio.get_writer('heat_map_py.gif', mode='I', duration=1) as writer:
    for (dirpath, dirnames, filenames) in os.walk(img_path):
        for filename in filenames:
            image = imageio.imread(img_path + '/' + filename)
            writer.append_data(image)
