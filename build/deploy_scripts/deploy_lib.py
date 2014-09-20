#! /usr/bin/python

import argparse
import shutil
import sys
from os.path import join, realpath, split, exists, getmtime, relpath
from os import makedirs
from re import match as re_match
import os

if __name__ == '__main__':
    #print (sys.argv)

    parser = argparse.ArgumentParser(prog='deploy_lib',description='Helper script for deploying library files')
    parser.add_argument('-l','--libname', help='Name of the library to copy', required=False)
    parser.add_argument('-s','--src', help='Source folder', required=True)
    parser.add_argument('-d','--dest', help='Destination directory', required=True)
    args = parser.parse_args()

    shared_lib_name = 'lib'+args.libname+'.so'
    re_string = '.*?\.(dll|pdb|exe|so)'
    if not args.libname is None:
        re_string = args.libname+re_string
    src_dir = realpath(args.src)
    for root, dirs, files in os.walk(args.src):
        dirs = [] # non-recursive search
        for file in files:
            if re_match(re_string, file) or args.libname == file or shared_lib_name == file:
                full_src_path = realpath(join(root, file))
                full_dest_path = realpath(join(args.dest,relpath(full_src_path, src_dir)))
                full_dest_dir, final_filename = split(full_dest_path)
                if not exists(full_dest_dir):
                    makedirs(full_dest_dir)
                if not exists(full_dest_path) or getmtime(full_src_path) > getmtime(full_dest_path):
                    print('Copying: '+full_src_path+' ---> '+full_dest_path)
                    shutil.copy2(full_src_path, full_dest_path)
