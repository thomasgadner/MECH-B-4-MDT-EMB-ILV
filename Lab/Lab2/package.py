#!/usr/bin/env python3

# pip3 install crc
import os
import crc
import sys
import struct
import binascii

#generate message in len | msg | crc16 format from string
def gen_package(msg:bytes) -> None:
	c = crc.Calculator(crc.Crc16.MODBUS)
	p = struct.pack('>b{}s'.format(len(msg)), len(msg)+2, msg)
	p += struct.pack('>H', c.checksum(p))
	s = binascii.hexlify(p, " ", 1).decode('ascii')
	print("package encoded to: {}".format(s))

def parse_package(msg:str) -> None:
	p = binascii.unhexlify(msg)
	if len(p) < 3:
		print("data malformed, package must have at least 3 bytes in size!");	
		sys.exit(os.EX_DATAERR)

	if p[0] != len(p) - 1:
		print("data malformed, size does not match package actual length");
		sys.exit(os.EX_DATAERR)

	c = crc.Calculator(crc.Crc16.MODBUS)
	c_expect = c.checksum(p[:-2])

	sz,msg,c_is = struct.unpack('>b{}sH'.format(len(p) - 3), p)

	if c_is != c_expect:
		print("data malformed, checksum is expected to be {:04x}, but was {:04x}".format(c_expect, c_is))
		sys.exit(os.EX_DATAERR)

	# package is valid, all checks passed
	print("package decoded to message: '{}'".format(msg.decode('ascii')))


def usage_out():
	print("Usage: {} [gen|parse] string_or_hexbytes".format(sys.argv[0]))
	print("    example: {} gen Foo ... will generate package with message Foo".format(sys.argv[0]))
	print("    example: {} parse 054261724909 ... will parse package and print its message 'Bar'".format(sys.argv[0]))
	sys.exit(os.EX_USAGE)
	

def main():
	if len(sys.argv) != 3:
		usage_out()

	string_or_hexbytes = sys.argv.pop()
	cmd = sys.argv.pop()

	string_or_hexbytes = string_or_hexbytes.encode('ascii')

	if cmd == "gen":
		gen_package(string_or_hexbytes)
	elif cmd == "parse":
		parse_package(string_or_hexbytes)
	else:
		# command not known
		usage_out()

	# success
	sys.exit(os.EX_OK)


if __name__ == "__main__":
	main()
