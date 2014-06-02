#! /usr/bin/python

import argparse
from os.path import join, realpath
from re import match as re_match
import os

parser = argparse.ArgumentParser(prog='copy_create_proto_files',description='Helper script for google protobuf files')
parser.add_argument('-s','--src', help='.proto source directory')
parser.add_argument('-d','--dest', help='.proto destination directory')

args = parser.parse_args()

for root, dirs, files in os.walk(args.src):
    for file in files:
        if (re_match(file, '.*?\.proto')):
            input_file = open(realpath(root, file), 'r')
            output_file = open(realpath(args.dest, file), 'wt')
            output_file.write('//Auto generated file. DO NOT EDIT!\n')
            output_file.write('option optimize_for=LITE_RUNTIME;\n')
            output_file.writelines(input_file.readlines())
            input_file.close()
            output_file.close()
