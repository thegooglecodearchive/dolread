import os, sys, shutil

from os.path import join
from difflib import Differ

base_path = os.path.abspath(os.path.dirname(sys.argv[0]))

def bs(path):
  return join(base_path, path)
  
html2txt_bin = bs(join('..', 'html2txt.exe'))
case_path = bs('case')

def differ(fn1, fn2):
  d = Differ()
  f1 = open(fn1)
  f2 = open(fn2)
  diff_res = d.compare(f1.readlines(), f2.readlines())
  for each in diff_res:
    if each[0] == '-' or each[0] == '+' or each[0] == '?':
      f1.close()
      f2.close()
      return True
  f1.close()
  f2.close()
  return False

def build_bin():
  os.chdir(bs('..'))
  os.system('make clean -f h2t.mak')
  ret = os.system('make -f h2t.mak')
  if (ret != 0):
    return ret
  os.chdir(base_path)
  return 0

if build_bin() != 0:
  print 'build failed.'
  sys.exit(1)
os.chdir(case_path)
cases = os.listdir('.')
n_total = n_succ = n_fail = 0
failed_cases = []
for c in cases:
  if not c.endswith('.htm'):
    continue
  name = c[0:c.index('.')]
  print 'running case %s' % name,
  res_filename = '%s.res' % name
  ref_filename = '%s.txt' % name
  os.system('%s %s > %s' % (html2txt_bin, c, res_filename))
  try:
    if differ(res_filename, ref_filename):
      failed_cases.append(name)
      n_fail += 1
      print 'fail'
    else:
      n_succ += 1
      print 'pass'
    n_total += 1
  except IOError:
  #ignore the text result file not existed error
    pass

print '''
===== Summary =====
total: %d
pass:  %d
fail:  %d(%s)
''' % (n_total, n_succ, n_fail, ','.join(failed_cases))

if n_fail == 0:
  print '===== ALL PASS ====='
else:
  print '===== PLEASE CHECK FAILED ====='
