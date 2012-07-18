import os.path
import sys
import argparse
import string
import re
from xml.dom.minidom import getDOMImplementation
from xml.etree import ElementTree as et

def stripLines(lines):
    for index, line in enumerate(lines):
        lines[index] = line.strip()
    return lines

def main():
    parser = argparse.ArgumentParser(prog="component_def_gen",description='Generates component definitions xml')
    parser.add_argument('-p','--proj', action='append' , help='Appends an Input project path to fixup')
    
    args = parser.parse_args()

    regexfile = open("project_unity_regex.txt")
    regexs = stripLines(regexfile.readlines())
    regexfile.close()
    
    buildplats = open("project_unity_build_platforms.txt")
    buildconfigs = stripLines(buildplats.readlines())
    buildplats.close()
    
    print "\nStarting fixup....\n"
    
    for p in args.proj:
        print 'fixing up project', p
        projxml = open(p)
        projtree = et.parse(projxml)
        
        relem = projtree.getroot()
        fileelems = relem.findall(".//File")
        
        for fe in fileelems:
            for rgex in regexs:
                if len(re.findall(rgex, fe.attrib["RelativePath"])) > 0 and fe.attrib["RelativePath"].find("Unity") == -1:
                    print '\t',fe.attrib["RelativePath"]
                    filecfgs = fe.findall("FileConfiguration")
                    if len(filecfgs) == 0:
                        for build in buildconfigs:
                            be = et.Element("FileConfiguration")
                            be.attrib["Name"] = build
                            be.attrib["ExcludedFromBuild"] = "true"
                            fe.append(be)
                    else:
                        for fc in filecfgs:
                            fc.attrib["ExcludedFromBuild"] = "true"
        
        projxml.close()
        
        projxml = open(p,'w')
        projtree.write(projxml)
        projxml.close()
        
if __name__ == '__main__':
    main()