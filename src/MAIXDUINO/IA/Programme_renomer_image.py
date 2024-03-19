# -*- coding: utf-8 -*-
"""
Created on Fri Mar 15 21:58:01 2024

@author: leova
"""

import os 

def rename_files(directory):
    files = os.listdir(directory)
    jpg_files = [file for file in files if file.endswith('.jpg') or file.endswith('.png')or file.endswith('.jfif')or file.endswith('.avif')]
    jpg_files.sort()
    count = 1
    
    for file in jpg_files: 
        new_name = f"Pano130_{count:02d}.jpg" #Changer le nom suivant le nom de l'image qu'on veut
        os.rename(os.path.join(directory, file), os.path.join(directory, new_name))
        count += 1

directory_path = "photo_IA"
rename_files(directory_path)
