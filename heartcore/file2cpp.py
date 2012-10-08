import sys
from getopt import getopt

optlist, args = getopt( sys.argv[1:], 'ci:o:d:' )
		
printComments = False
for o in optlist:
	if o[0] == '-i':
		inputfile = o[1]
	if o[0] == '-o':
		outputfile = o[1]
	if o[0] == '-d':
		dataprefix = o[1]
	if o[0] == '-c':
		printComments = True
		
ofile = open( outputfile, 'w' )
ofile.write( '/*\nFile Generated with file2cpp.py. DO NOT EDIT\nSource File: ' + inputfile + '\n*/\n\n' )

ifile = open( inputfile, 'rb' )
data = ifile.read()

ofile.write( '#ifndef __'+dataprefix+'__\n#define __'+dataprefix+'__\n\n' )
ofile.write( 'extern "C" {\n' )
ofile.write( 'const unsigned long ' + dataprefix + '_size = ' + str(len( data )) + ';\n' )
ofile.write( 'const unsigned char ' + dataprefix + '[] = \n{\n' )

i = 0
for c in data:
	ofile.write( hex( ord( c ) ) + ', ' )
	i += 1
	if i == 40:
		ofile.write( '\n' )
		i = 0

ofile.write( '\n};\n}//extern "C"\n#endif//__'+dataprefix+'__\n' )
ifile.close()

if printComments:
	ifile = open( inputfile, 'r' )
	ofile.write( '/************************************************************************\n\n' )
	for line in ifile.readlines():
		ofile.write( line )
	ofile.write( '************************************************************************/\n\n' )
	ifile.close()
	
ofile.close()