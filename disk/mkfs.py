#!/bin/python2

import os, sys, struct, StringIO

MAGIC_NUMBER = 0x55D0AFDE
number_of_entries = 0

def list_directory(full_path, local_directory, heado, bino):
	print "Packing directory ", full_path, " as ", local_directory
	items_in_directory = os.listdir(full_path)
	for item in items_in_directory:                       
		fpath = full_path + os.path.sep + item
		if os.path.isdir(fpath):
			list_directory(fpath, local_directory + "/" + item if local_directory != "" else item, heado, bino)
		else:
			ld = local_directory + "/" if local_directory != "" else ""
			initpath = b"%s%s\x00" % (ld, item)
			if (len(initpath) > 255):
				print "File ", item, " not packed due to path being too long"
				continue
			initpath = b"%-256s" % initpath
			print "Packing file: ", item
			global number_of_entries
			number_of_entries += 1
			ifl = open(fpath, "rb")
			d = ifl.read();
			pos = bino.tell()
			bino.write(d)                        
			heado.write(struct.pack("II", pos, len(d)))
			heado.write(initpath)
       

def pack_data(directory):
	f = StringIO.StringIO()
	hf = StringIO.StringIO()
	hf.seek(12)
	pos = list_directory(directory, "", hf, f)

	f.seek(0)
	hf.write(f.read())

	hf.seek(0)
	hf.write(struct.pack("III", MAGIC_NUMBER, number_of_entries, 12+(number_of_entries*264)))
	return hf

if __name__ == "__main__":
	if len(sys.argv) < 3:
		print "mkfs.py -- usage"
		print "  mkfs.py <input directory> <output file>"
	else:
		io = pack_data(sys.argv[1])
		f = open(sys.argv[2], "wb")
		io.seek(0)
		f.write(io.read())
		f.close()
  
