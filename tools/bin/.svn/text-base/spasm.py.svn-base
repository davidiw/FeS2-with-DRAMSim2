#!/usr/bin/python

from optparse import OptionParser
import os
import sys, re

descripText = """Expands strings of the form ASSEMBLE("x86 insn...") into their resulting binary representation"""

# parse command-line flags
parser = OptionParser( usage="%prog [options] file.cpp",
                       description=descripText )

parser.add_option( "-d", "--debug", dest="debug", default=False, action="store_true",
                   help="Print debug messages to stderr." )

opts, args = parser.parse_args()

assert len(args) == 1, "Incorrect usage: run with --help for usage info."
cppFile = args[0]
assert os.path.isfile( cppFile )

asmRegex = re.compile( r'ASSEMBLE[(]"(?P<asm>[^"]*)"[)]' )
bytesRegex = re.compile( r'(?P<addr>[ 0-9a-fA-F]*):(?P<bytes>([ \t][0-9a-fA-F]{2})+)' )

for line in file( cppFile ).readlines():
    matches = asmRegex.search( line )
    if matches is None:
        print line,
        continue

    # found a match
    asm = matches.group( 'asm' )
    asm = asm.replace("$", r"\$")
    assembler = os.path.join( os.path.dirname(sys.argv[0]), "assemble.sh" )
    cmd = assembler + ' "%s"' % asm
    
    if opts.debug:
        sys.stderr.write( "DEBUG: running shell command: " + cmd + "\n" )
        
    r, pipe, e = os.popen3( cmd )
    pipedLines = pipe.readlines()
    hexdumpLine = pipedLines[0].strip()
    hexdumpMatches = bytesRegex.search( hexdumpLine )
    if hexdumpMatches is None:
      sys.stderr.write("ERROR: assembled instr not in right format:\n")
      sys.stderr.write(asm + "\n")
      for pipedLine in pipeLines:
          sys.stderr.write(pipedLine + "\n")
      sys.stderr.write(hexdumpLine + "\n")
      sys.exit(1)
    bytes = hexdumpMatches.group( 'bytes' ).strip()
    bytes = bytes.split()
    numBytes = len( bytes )
    bytes = [ r'"\x' + b + '"' for b in bytes ]
    bytes = " ".join(bytes)

    if opts.debug:
        sys.stderr.write( "DEBUG: assembled bytes: " + bytes + "\n" )

    arg = "(byte*)" + " (" + bytes + "), " + str( numBytes )
    line = asmRegex.sub( arg, line )
    print line,
    pipe.close()
    r.close()
    e.close()
