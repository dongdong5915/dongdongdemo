#!/usr/bin/python2.7
#
#  nt72668_burn_img.py
#
#  Author: 	dd xa00086
#  Created: 	May 09, 2014
#  Copyright: Novatek Inc.
#
#
from Tkinter import *
import sys
import struct
import os
import hashlib
NVTFW_WHOLE_IMG_SIGN = "BIMG"
NVTFW_IMG_NAME_LEN = 16

NVTFW_OUTPUT_IMG_NAME = "NT667.bin"
NVTFW_OUTPUT_TMP_HEADER = ".tmpheader"
NVTFW_OUTPUT_TMP_NAME = ".tmpimg"


NVTFW_PER_IMG_FMT = "{0}sII".format(NVTFW_IMG_NAME_LEN)

NVTFW_VAILD_IMG_NAME = ["ddrcfg","mloader","stbc_emmc","stbc","logo","ker0","ker1","database","config","mid_base","soc_lib","system"]

NVTFW_VAILD_FIELD = ["startpos","file_name"]

def show_msg(msg):
	if sys.platform == "win32":
		root = Tk()
		root.title("nt72668 burn img tool")
		Message(root,text=msg,bg="royalblue",fg="ivory",relief=GROOVE).pack(padx=10,pady=10)
		mainloop()
	else:
		print(msg)

class nvtfw_per_img_obj:
	def __init__(self):
		self.img_name = ""
		self.flen = 0
		self.startpos = 0
		self.file_name = ""

	def __repr__(self):
		mystr = "img_name = " + self.img_name + "\n"
		mystr += "flen = " + str(self.flen) + "\n"
		mystr += "startpos = " + str(self.startpos) + "\n"
		mystr += "file_name = " + self.file_name + "\n"
		return mystr

	def __str__(self):
		return repr(self)

	def set_file_name(self, file_name):
		self.file_name = file_name
		cwd = os.path.dirname(sys.argv[0])
		self.flen = os.path.getsize(os.path.join(cwd,self.file_name))

	def set_img_name(self, img_name):
		self.img_name = img_name

	def set_startpos(self, startpos):
		self.startpos = int(startpos)

	def gen_image_header(self, fout):
		cwd = os.path.dirname(sys.argv[0])
		hdr = struct.pack(NVTFW_PER_IMG_FMT,self.img_name,self.flen,self.startpos)
		fout.write(hdr)

	def gen_image(self, fout):
		cwd = os.path.dirname(sys.argv[0])
		fin = open(os.path.join(cwd,self.file_name),"rb")
		if self.flen%4096 == 0:
			fout.write(fin.read())
		else :
			perimgfilldatlen = 4096 - (self.flen%4096)
			filldat = struct.pack("{0}s".format(perimgfilldatlen),'0')
			fout.write(fin.read())
			fout.write(filldat)
		fin.close()

class nvtfw_whole_img_obj:
	def __init__(self):
		self.sign = NVTFW_WHOLE_IMG_SIGN
		self.pimg_tbl = []
		self.cfg_fd = 0

	def check_field_valid(self,field_name):
		if field_name.strip() in NVTFW_VAILD_FIELD:
			return True
		else :
			return False

	def check_img_name_valid(self, img_line):
		img_name = img_line.strip("[] \n")
		if img_name in NVTFW_VAILD_IMG_NAME:
			return True
		else:
			return False

	def parse_cfg_file(self, cfg_fname):
		cwd = os.path.dirname(sys.argv[0])
		self.cfg_fd = open(os.path.join(cwd,cfg_fname),"rw")
		img_name = ""
		file_name = ""
		startpos = ""

		while True :

			line = self.cfg_fd.readline()
			if line == '' :
				break

			if line[0] == '#' or len(line.strip()) == 0 :
				continue

			if line.startswith('[') and self.check_img_name_valid(line):
				img_name = line

			if line.startswith("file_name"):
				file_name = line

			if line.startswith("startpos"):
				startpos = line

			if len(img_name) > 0 and len(file_name) > 0 and len(startpos) > 0 :
				#parse file name first, if file name is empty, skip to next
				tmp = file_name.split("=")
				if len(tmp[1].strip("\n ")) == 0:
					img_name = ""
					file_name = ""
					startpos = ""
					continue
				else:
					pimg = nvtfw_per_img_obj()
					pimg.set_img_name(img_name.strip("[] \n"))

					tmp = startpos.split("=")
					pimg.set_startpos(tmp[1].strip())

					tmp = file_name.split("=")
					pimg.set_file_name(tmp[1].strip())

					self.pimg_tbl.append(pimg)

					img_name = ""
					file_name = ""
					startpos = ""

		self.cfg_fd.close()

	def gen_update_img(self):
		cwd = os.path.dirname(sys.argv[0])

		fout = open(os.path.join(cwd,NVTFW_OUTPUT_IMG_NAME),"wb+")

		ftmpheader = open(os.path.join(cwd,NVTFW_OUTPUT_TMP_HEADER),"wb+")

		ftmp =open(os.path.join(cwd,NVTFW_OUTPUT_TMP_NAME),"wb+")


		for i in self.pimg_tbl:
			i.gen_image_header(ftmpheader)

		ftmpheader.flush()

		ftmpheader.seek(0, os.SEEK_END)
		ftmpheader_flen = ftmpheader.tell()

		ftmpheader.seek(0, os.SEEK_SET)

		hdr = struct.pack("4s",self.sign)

		ftmpheader_fill = 4096 - 4 - ftmpheader_flen

		fout.write(hdr)
		fout.write(ftmpheader.read())

		tdr = struct.pack("{0}s".format(ftmpheader_fill),'0')
		fout.write(tdr)

		for k in self.pimg_tbl:
			k.gen_image(ftmp)

		ftmp.flush()
		ftmp.seek(0,os.SEEK_SET)
		fout.write(ftmp.read())
		ftmpheader.close()
		ftmp.close()
		fout.close()

		os.remove(os.path.join(cwd,NVTFW_OUTPUT_TMP_NAME))
		os.remove(os.path.join(cwd,NVTFW_OUTPUT_TMP_HEADER))
wimg = nvtfw_whole_img_obj()
wimg.parse_cfg_file(sys.argv[1])
wimg.gen_update_img()
