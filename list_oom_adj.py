#!/usr/bin/env python3

import sys, os, re

from pwd import getpwuid
from grp import getgrgid


number = re.compile("[+-]?\d+")

l = []

for d in os.listdir("/proc"):
 try:
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
 except FileNotFoundError:
  pass

ll = []
if len(l) > 0:
 prev = l[0]
 count = 1
 for v in l[1:]:
  if v != prev:
   if count > 1:
    ll.append(("%s*%d"%(prev[0], count), prev[1], prev[2], prev[3], count))
   else:
    ll.append((prev[0], prev[1], prev[2], prev[3], count))
   prev = v
   count = 1
  else:
   count += 1

l = ll

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
 fmt = "{{0:<{0}}}  {{1:<{1}}}  {{2:<{2}}}  {{3:>8d}}".format(max_cmd, max_usr, max_grp)
 out = fmt.format(v[0], v[1], v[2], v[3])
 print(out)

