#! /usr/bin/python

import argparse
import shutil
from os.path import join, realpath, split, exists, getmtime
from re import match as re_match
import os

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='copy_create_proto_files',description='Helper script for google protobuf files')
    parser.add_argument('-l','--libname', help='Name of the library to copy', required=False)
    parser.add_argument('-s','--src', help='Source folder', required=True)
    parser.add_argument('-d','--dest', help='Destination directory', required=True)
    args = parser.parse_args()

    re_string = '.*?\.(dll|pdb|exe)'
    if not args.libname is None:
        re_string = args.libname+re_string
    src_dir = realpath(args.src)
    for root, dirs, files in os.walk(args.src):
        for file in files:
            if re_match(re_string, file):
                full_src_path = realpath(join(root, file))
                src_path = realpath(full_src_path[len(src_dir)+1:])
                full_dest_path = realpath(join(args.dest, src_path))
                if not exists(full_dest_path) or getmtime(full_src_path) > getmtime(full_dest_path):
                    print('Copying: '+full_src_path+' ---> '+full_dest_path)
                    shutil.copy2(full_src_path, full_dest_path)