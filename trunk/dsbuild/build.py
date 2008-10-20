import os, sys, shutil
from os.path import join, exists
from difflib import Differ
from optparse import OptionParser

build_path = os.path.abspath(os.path.dirname(sys.argv[0]))
build_src_path = join(build_path, 'source')
if not os.path.isdir(build_src_path):
  os.mkdir(build_src_path)
build_data_path = join(build_path, 'data')
if not os.path.isdir(build_data_path):
  os.mkdir(build_data_path)
  
def differ(fn1, fn2):
  d = Differ()
  f1 = open(fn1)
  f2 = open(fn2)
  for each in d.compare(f1.readlines(), f2.readlines()):
    if each[0] == '-' or each[0] == '+' or each[0] == '?':
      f1.close()
      f2.close()
      return True
  f1.close()
  f2.close()
  return False

#copy all required source files
data_files = ['asc_font_matrix_16.c', 'chn_font_matrix_16.c', 'asc_font_matrix_14.c', 'chn_font_matrix_14.c', 'uni_tab.c']
src_path = join(build_path, '..', 'src')
for f in data_files:
  src = join(src_path, f)
  dst = join(build_data_path, f)
  if ((not os.path.isfile(dst)) or differ(src, dst)):
    shutil.copyfile(src, dst)

common_files = ['reader_core.c', 'reader_util.c', 'reader_gui.c', 'reader_file.c', 
                'reader_str_res.c', 'reader_color.c', 'reader_about.c', 'reader_mmi.c', 
                'reader_time.c', 'reader_option.c', 'reader_sav.c', 'reader_path.c']
for f in common_files:
  src = join(src_path, f)
  dst = join(build_src_path, f)
  if ((not os.path.isfile(dst)) or differ(src, dst)):
    shutil.copyfile(src, dst)

nds_files = ['main.c', 'reader_mmi_dev.c', 'reader_misc_dev.c']
for f in nds_files:
  src = join(src_path, 'nds', f)
  dst = join(build_src_path, f)
  if ((not os.path.isfile(dst)) or differ(src, dst)):
    shutil.copyfile(src, dst)

fsal_files = ['fsal_nds.c']
fsal_path = join(build_path, '..', 'fat')
for f in fsal_files:
  src = join(fsal_path, f)
  dst = join(build_src_path, f)
  if ((not os.path.isfile(dst)) or differ(src, dst)):
    shutil.copyfile(src, dst)

#place the logo picture to right place
#check the corresponding value of $ICON in Makefile
logoes = ['DR_Logo.bmp', 'logo_wifi.bmp']
for logo in logoes:
  pic_dest = "C:\\%s" % logo
  if not exists(pic_dest):
    pic_path = join(build_path, '..', 'doc', logo)
    shutil.copyfile(pic_path, pic_dest)

#copy fscr dldi driver
fscr_dest = join(build_path, 'fcsr.dldi')
dldi_path = join('..', 'tools', 'dldi_drv')
if not exists(fscr_dest):
  fscr_path = join(dldi_path, 'fcsr.dldi')
  shutil.copyfile(fscr_path, fscr_dest)

os.chdir(build_path)
cmd = 'make -f dr.mak'
print cmd
sys.exit(os.system(cmd))

