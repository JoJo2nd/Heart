#! /usr/bin/python

import argparse
import shutil
from os.path import join, realpath, split, exists, getmtime, relpath
from re import match as re_match
import os

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='copy_create_proto_files',description='Helper script for google protobuf files')
    parser.add_argument('-r','--regex', help='RegEx to match copy on copy')
    parser.add_argument('-s','--src', help='Source folder', required=True)
    parser.add_argument('-d','--dest', help='Destination directory', required=True)
    args = parser.parse_args()

    re_string = args.regex
    src_dir = realpath(args.src)
    for root, dirs, files in os.walk(args.src):
        for file in files:
            if re_match(re_string, file):
                full_src_path = realpath(join(root, file))
                full_dest_path = realpath(join(args.dest,relpath(full_src_path, src_dir)))
                if not exists(full_dest_path) or getmtime(full_src_path) > getmtime(full_dest_path):
                    print('Copying: '+full_src_path+' ---> '+full_dest_path)
                    shutil.copy2(full_src_path, full_dest_path)