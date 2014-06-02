#! /usr/bin/python

import argparse
import subprocess
import sys
import os
from re import match as re_match
from os import walk
from os.path import realpath
from os.path import join as path_join

class Unbuffered(object):
   def __init__(self, stream):
       self.stream = stream
   def write(self, data):
       self.stream.write(data)
       self.stream.flush()
   def __getattr__(self, attr):
       return getattr(self.stream, attr)

sys.stdout = Unbuffered(sys.stdout)

def checkForNewSource(in_path):
    exclude_directories = ['.svn','.git']
    full_paths = []
    for root, dirs, files in walk(in_path):
        for excluded_dir in exclude_directories: 
            if excluded_dir in dirs:
                dirs.remove(excluded_dir)
        for file in files:
            full_paths.append(realpath(path_join(root, file))+'\n')
    try:
        build_list = open('build.timestamp.txt', 'r')
    except:
        return True

    prev_paths = build_list.readlines()
    build_list.close()
    return full_paths != prev_paths

def writeBuildTimestamp(in_path):
    exclude_directories = ['.svn','.git']
    full_paths = []
    for root, dirs, files in walk(in_path):
        for excluded_dir in exclude_directories: 
            if excluded_dir in dirs:
                dirs.remove(excluded_dir)
        for file in files:
            full_paths.append(realpath(path_join(root, file))+'\n')
    build_list = open('build.timestamp.txt', 'w')
    build_list.writelines(full_paths)
    build_list.close()


parser = argparse.ArgumentParser(prog="cmake_build",description='Helper script for cmake compiles')
parser.add_argument('-r','--root', help='root source directory')
parser.add_argument('-p','--path', help='cmake build directory')

args = parser.parse_args()

heart_src_path = realpath(path_join(args.path,'../heart/src'))
heart_output_path = realpath(path_join(args.path,'../heart/src'))

print('Checking project is up-to-date...',)
if checkForNewSource(heart_src_path):
    print('re-generating project.')
    subprocess.call(['python', 'c_cpp_unity_creator.py', 
        '-d', heart_src_path, 
        '-x', 'unix', 
        '-x', 'posix',
        '-e', '.+\.(h|hpp|gitignore|py|lua|txt|inl)',
        '-e', '.+(.unity[\d*].cpp)', 
        '-e', '.+([U|u]nity.cpp)', 
        '-o', heart_output_path, '-n', 'heart'])
    subprocess.call(["cmake", args.path, '-DUNITY_BUILD=ON'])
    writeBuildTimestamp(heart_src_path)
else:
    print('project is up-to-date')

print('Starting build...')

proc = subprocess.Popen(["cmake", "--build", args.path], stdout=subprocess.PIPE, universal_newlines=True)
while not proc.poll():
    line = proc.stdout.readline()
    if line != '' and not line.isspace():
        #todo fix to handle forward slash...?
        match = re_match('(\d+>\s*)(?:\.\.\\\\)*(.*)\((\d+)\)(.*)', line)
        if match:
            corrected_path = realpath(path_join(args.root, match.group(2)))
            print(match.group(1)+corrected_path+'('+match.group(3)+')'+match.group(4))
        else:
            print(line.strip())