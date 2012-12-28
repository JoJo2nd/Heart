import os.path
import sys
import argparse
import string
import re
from xml.dom.minidom import getDOMImplementation
from xml.etree import ElementTree as et
import xml.etree

def stripLines(lines):
    for index, line in enumerate(lines):
        lines[index] = line.strip()
    return lines

def fixupVS2010(args, regexs, buildconfigs):
    et.register_namespace("","http://schemas.microsoft.com/developer/msbuild/2003")
    
    ## VS 2010 format
    ##<ClCompile Include="..\..\..\..\heart\gwen\src\Anim.cpp">
    ##  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</ExcludedFromBuild>
    ##  <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</ExcludedFromBuild>
    
    print("\nStarting fixup of 2010 project....\n")
    
    for p in args.proj:
        print 'fixing up project', p
        projxml = open(p)
        projtree = et.parse(projxml)
        
        relem = projtree.getroot()
        fileelems = []
        fileelemstmp = relem.findall(".//{http://schemas.microsoft.com/developer/msbuild/2003}ClCompile")
        for fe in fileelemstmp:
            if not fe.get("Include") == None:
                fileelems.append(fe)
        
        for fe in fileelems:
            for rgex in regexs:
                if len(re.findall(rgex, fe.attrib["Include"])) > 0 and fe.attrib["Include"].find("Unity") == -1:
                    print '\t',fe.attrib["Include"]
                    filecfgs = fe.findall("ExcludedFromBuild")
                    if len(filecfgs) == 0:
                        for build in buildconfigs:
                            be = et.Element("ExcludedFromBuild")
                            be.attrib["Condition"] = "'$(Configuration)|$(Platform)'=='"+build+'\''
                            be.text = "true"
                            fe.append(be)
                    else:
                        for fc in filecfgs:
                            fc.text = "true"
        
        projxml.close()
        
        projxml = open(p,'w')
        projtree.write(projxml,encoding="utf-8", xml_declaration=True, method="xml")
        projxml.close()
    
def fixupVS2008(args, regexs, buildconfigs):
    print("\nStarting fixup of 2008 project....\n")
    
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
        projtree.write(projxml,encoding="utf-8", xml_declaration=True, method="xml")
        projxml.close()
    
def main():
    parser = argparse.ArgumentParser(prog="component_def_gen",description='Generates component definitions xml')
    parser.add_argument('-p','--proj', action='append' , help='Appends an Input project path to fixup')
    parser.add_argument('-m','--mode', help='Selects between "vs2008" mode and "vs2010" mode')
    parser.add_argument('-r','--regexfile', help='Text file with a list of RegEx expressions to match project file names against')
    parser.add_argument('-b','--builds', help='text file with list of build platforms to match')
    
    args = parser.parse_args()

    regexfile = open(args.regexfile)#"project_unity_regex.txt")
    regexs = stripLines(regexfile.readlines())
    regexfile.close()
    
    buildplats = open(args.builds)#"project_unity_build_platforms.txt")
    buildconfigs = stripLines(buildplats.readlines())
    buildplats.close()
    
    if args.mode == "vs2010":
        fixupVS2010(args, regexs, buildconfigs)
    elif args.mode == "vs2008":
        fixupVS2008(args, regexs, buildconfigs)
    else:
        print("unknown project mode")

        
if __name__ == '__main__':
    main()