#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import csv

state = "idle"

colindex= {}
colnames = []
cols = []

while True:
    s_in = input()
    s_in = s_in.split()

    if s_in[0]=="X":
        break

    if state=="idle":

        if s_in[0]=="cols":
            cols = []
            colnames = []
            colindex = {}
            for c in s_in[1:]:
                colnames.append(c)
                cols.append([])
            for i, colname in enumerate(colnames):
                colindex[colname] = i

        elif s_in[0]=="data":
            state="data"

        elif s_in[0]=="csv":
            fname = s_in[1]
            colused = []
            for c in s_in[2:]:
                colused.append(c)
            with open(fname, 'w') as f:
                writer = csv.writer(f)
                writer.writerow(colused)
                for i in range(len(cols[0])):
                    r = []
                    for name in colused:
                        r.append(cols[colindex[name]][i])
                    writer.writerow(r)

        elif s_in[0]=="plot":
            xname = ""
            options = {}
            form = s_in[1].split(',')
            fig, ax = plt.subplots(int(form[0]), int(form[1]))
            if type(ax) is not np.ndarray:
                ax = [ax]
            plotnr = 0

            for t in s_in[2:]:

                if t.startswith("x:"):
                    xname = t.split(':')[1]
                    options = {}
                    plotnr += 1

                elif t.startswith("y:"):
                    t = t.split(':')
                    options[t[1]] = {}
                    o_current = None
                    for o in t[2:]:
                        if o_current is not None:
                            options[t[1]][o_current] = o
                            o_current = None
                        else:
                            if o=="color":
                                o_current = "color"
                            elif o=="label":
                                o_current = "label"

                elif t=="p":
                    sp = ax[plotnr-1]
                    for yname, opt in options.items():
                        line, = sp.plot(cols[colindex[xname]], cols[colindex[yname]], label=yname)
                        for oname, oval in opt.items():
                            if oname=="color":
                                line.set_color(oval)
                            elif oname=="label":
                                line.set_label(oval)
                    sp.set_xlabel(xname)
                    sp.legend()

            plt.show()


    elif state=="data":

        if s_in[0]=="e":
            state="idle"
            continue

        if len(s_in)<len(colnames):
            print("ERROR: incoming data not in right format. need more columns")
            exit()

        for i, d in enumerate(s_in):
            if i>= len(colnames):
                break
            cols[i].append(float(d))

