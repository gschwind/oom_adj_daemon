#!/usr/bin/env python3

import sys, os, re

from pwd import getpwuid
from grp import getgrgid


number = re.compile("[+-]?\d+")

l = []

for d in os.listdir("/proc"):
 if number.match(d):
  f = open(os.path.join("/proc", d, "oom_score_adj"), "r")
  adj = int(f.read())
  f.close()
  f = open(os.path.join("/proc", d, "stat"), "r")
  cmd = f.read().split(' ')[1]
  f.close()
  st = os.stat(os.path.join("/proc", d))
  us = getpwuid(st.st_uid)
  gr = getgrgid(st.st_gid)
  if len(cmd) == 0:
   cmd = "unknown#%s"%(d,)
  l.append((cmd, us.pw_name, gr.gr_name, adj))

max_cmd = 0
max_usr = 0
max_grp = 0
for v in l:
 if len(v[0]) > max_cmd:
  max_cmd = len(v[0])
 if len(v[1]) > max_usr:
  max_usr = len(v[1])
 if len(v[2]) > max_grp:
  max_grp = len(v[2])

for v in l:
 fmt = "{{0:<{0}}}  {{1:<{1}}}  {{2:<{2}}}  {{3:>8d}}".format(max_cmd, max_usr, max_grp).format(v[0], v[1], v[2], v[3])
 print(fmt)

