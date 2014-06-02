#! /usr/bin/python

import argparse
from os.path import join, realpath
from re import match as re_match
import os

parser = argparse.ArgumentParser(prog='c_cpp_unity_creator',description='Tool to create unity build files from C/C++ source files')
parser.add_argument('-d','--directory', action='append', help='C/C++ source directory. Recursive.')
parser.add_argument('-x','--excludedirectory', action='append', help='names of directories to exclude from the search')
parser.add_argument('-e','--excludefile', action='append', help='RegEx of file names to exclude')
parser.add_argument('-o','--output', help='C/C++ output directory.')
parser.add_argument('-n','--name', help='C/C++ output name prefix.')

args = parser.parse_args()

unity_file_list = []

def main():
    for unity_dir in args.directory:
        for root, dirs, files in os.walk(unity_dir):
            for excluded_dir in args.excludedirectory: 
                if excluded_dir in dirs:
                    dirs.remove(excluded_dir)
            for file in files:
                full_path = realpath(join(root, file))
                exclude = False
                for regex in args.excludefile:
                    if re_match(regex, full_path):
                        exclude = True
                        break

                if not exclude:
                    unity_file_list.append(full_path)

    output_path = join(args.output, args.name+".unity.cpp")
    output_file = open(output_path,'w')
    output_file.write('/** Auto generated file. Do not edit. **/\n')
    for unity_file in unity_file_list:
        output_file.write('#include "'+unity_file+'"\n')
    output_file.close()


if __name__ == '__main__':
    main()