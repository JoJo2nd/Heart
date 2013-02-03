import argparse
from struct import unpack

def main():
    parser = argparse.ArgumentParser(prog="dds_to_cpp",description='Creates a C++ array to embed in exe')
    parser.add_argument('-i','--input', help='Input DDS to process')
    parser.add_argument('-o','--output', help='Output file to write')
    parser.add_argument('-d','--dataname', help='Name of the array data')
    
    args = parser.parse_args()

    ## DDS header
    # DWORD           dwSize;
    # DWORD           dwFlags;
    # DWORD           dwHeight;
    # DWORD           dwWidth;
    # DWORD           dwPitchOrLinearSize;
    # DWORD           dwDepth;
    # DWORD           dwMipMapCount;
    # DWORD           dwReserved1[11];
    # DDS_PIXELFORMAT ddspf;
    # DWORD           dwCaps;
    # DWORD           dwCaps2;
    # DWORD           dwCaps3;
    # DWORD           dwCaps4;
    # DWORD           dwReserved2;
    
    with open(args.input, "rb") as f:
        ddsmagicnum, size, flags, height, width, pitch, depth, mipcount, = unpack('IIIIIIII', f.read(32))
        f.read(11*4)
        pfsize, pfflags, pfFourCC, pfbitcount, pfRmask, pfGmask, pfBmask, pfAmask, = unpack('IIIIIIII', f.read(32))
        if (ddsmagicnum != 0x20534444):
            print "Input file is not DDS file"
            return
        caps, caps2, caps3, caps4, reserved = unpack('IIIII', f.read(20))
        realpitch=(width * pfbitcount+7) / 8
        print "W:", width, "H:", height, "P:", pitch, "RP:", realpitch, "BitPP:", pfbitcount
        # TODO: check for DX10 header and skip?
        with open(args.output, "w") as w:
            w.write("/*******************************************************\n")
            w.write("  Auto-Generated file. Do not edit\n");
            w.write("*******************************************************/\n\n");
            w.write("extern \"C\" {\n")
            w.write("const size_t %s_width = %d;\n" % (args.dataname, width))
            w.write("const size_t %s_height = %d;\n" % (args.dataname, height))
            w.write("const size_t %s_pitch = %d;\n" % (args.dataname, realpitch))
            w.write("const size_t %s_bits_per_pixel = %d;\n" % (args.dataname, pfbitcount))
            w.write("const size_t %s_data_len = %d;\n" % (args.dataname, (height*realpitch*pfbitcount+7)/8))
            w.write("const char %s_data[] = {\n" % args.dataname)
            for y in range(height):
                for x in range(realpitch):
                    byte, = unpack('B', f.read(1))
                    w.write("0x%02X" % byte)
                    w.write(", ")
                w.write("\n")
            w.write("};\n")
            w.write("} //extern \"C\"\n")
            
if __name__ == '__main__':
    main()