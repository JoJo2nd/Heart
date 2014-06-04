#! /usr/bin/python

import argparse
from os.path import join, realpath, split
from re import match as re_match
import os

def writeFile(root, in_file, dest):
    in_file = str(in_file)
    out_includes = []
    input_file = open(realpath(join(root, in_file)), 'r')
    output_file = open(realpath(join(dest, in_file)), 'wt')
    output_file.write('//Auto generated file. DO NOT EDIT!\n')
    output_file.write('option optimize_for=LITE_RUNTIME;\n')
    for line in input_file.readlines():
        match = re_match('\s*?import\s+?\"(.+?)\"', line)
        if match:
            out_includes.append(match.group(1))
        output_file.write(line)
    input_file.close()
    output_file.close()
    return out_includes

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='copy_create_proto_files',description='Helper script for google protobuf files')
    parser.add_argument('-s','--src', help='.proto source file')
    parser.add_argument('-d','--dest', help='.proto destination directory')

    args = parser.parse_args()
    print(args.src)
    proto_root, proto_file = split(args.src)
    files_to_process = [proto_file]

    while len(files_to_process) != 0:
        files_to_process.extend(writeFile(proto_root, files_to_process[0], args.dest))
        files_to_process = files_to_process[1:]

