#!/usr/bin/env python3

import sys, os, re

number = re.compile("[+-]?\d+")

for d in os.listdir("/proc"):
 if number.match(d):
  f = open(os.path.join("/proc", d, "oom_score_adj"), "r")
  adj = int(f.read())
  f.close()
  f = open(os.path.join("/proc", d, "cmdline"), "r")
  cmd = os.path.basename(f.read().split('\x00')[0])
  f.close()
  if len(cmd) == 0:
   continue
  print((cmd, adj))

