#!/usr/bin/python
# -*- coding: utf-8 -*-

#  Copyright (c) 2019, Fudan University

import random

fout = open("data.txt","w")
fout.write("10000\n")
for i in range(10000):
    x = random.uniform(-100,100)
    y = random.uniform(-100,100)
    data = random.randint(-1000,1000)
    fout.write("%.3f %.3f\t%d\n" % (x, y, data))
fout.close()
