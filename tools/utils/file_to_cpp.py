import argparse
from os import stat
from struct import unpack

def main():
    parser = argparse.ArgumentParser(prog="file_to_cpp",description='Creates a C++ array to embed in exe')
    parser.add_argument('-i','--input', help='Input file to process')
    parser.add_argument('-o','--output', help='Output .cpp to write')
    parser.add_argument('-d','--dataname', help='Name of the array data')
    
    args = parser.parse_args()
    
    with open(args.input, "rb") as f:
        data_len = stat(args.input).st_size
        with open(args.output, "w") as w:
            w.write("/*******************************************************\n")
            w.write("  Auto-Generated file. Do not edit\n");
            w.write("*******************************************************/\n\n");
            w.write("extern \"C\" {\n")
            w.write("const size_t %s_data_len = %d;\n" % (args.dataname, data_len))
            w.write("const char %s_data[] = {\n" % args.dataname)
            for y in range(data_len):
                byte, = unpack('B', f.read(1))
                w.write("0x%02X" % byte)
                w.write(", ")
                if (y+1) % 50 == 0:
                    w.write("\n")
            w.write("0x0};\n")
            w.write("} //extern \"C\"\n")
            
if __name__ == '__main__':
    main()