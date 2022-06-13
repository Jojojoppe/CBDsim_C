#!/usr/bin/env python3
from turtle import update
import matplotlib.pyplot as plt
import numpy as np
import signal

windows = {}
cols = {}
colformat = []

_sigINT = False

def _sighandle(sig, frame):
    global _sigINT
    if(sig==signal.SIGINT):
        _sigINT=True

signal.signal(signal.SIGINT, _sighandle)

# Update all graphs and redraw them
def dataupdate():
    for wk, win in windows.items():
        for pi, plot in win["plots"].items():
            for i in range(len(plot["lines"])):
                line = plot["lines"][i]
                colx, coly = plot["linecols"][i]
                line.set_xdata(colx)
                line.set_ydata(coly)
                if len(colx)>0 and len(coly)>0:
                    plot["lastlines"][i].set_xdata([colx[-1]])
                    plot["lastlines"][i].set_ydata([coly[-1]])
            plot["ax"].relim()
            plot["ax"].autoscale_view(tight=True, scalex=plot["autoscale"][0], scaley=plot["autoscale"][1])
        win["fig"].canvas.draw()
        win["fig"].canvas.flush_events()

while True and not _sigINT:
    # Get input
    string_in = input()
    # Tokenize
    t_in = string_in.split()
    if len(t_in)<1:
        continue

    # print("VIZ:", t_in)

    cmd = t_in[0]

    # Stop plotter
    if cmd=="quit":
        break
    elif cmd=="stop":
        plt.show(block=True)
        break

    # Reset ploter
    if cmd=="reset":
        plt.close('all')
        windows = {}
        cols = {}
        colformat = []
        realtime = None
        realtimecounter = 0

    # Create plot window
    # wind <win_name> [title:<>]
    elif cmd=="wind":
        win_name = t_in[1]
        windows[win_name] = {
            "fig" : plt.figure(),
            "plots" : {},
        }
        # Loop over extra options
        for o in t_in[2:]:
            o = o.split(':')
            o_name = o[0]
            if o_name=="title":
                windows[win_name]["fig"].suptitle(o[1].replace('__', ' '))
        windows[win_name]["fig"].show()
        dataupdate()

    # Create plot in window
    # plot <win_name> <plt_name> <loc> [title:<>] [xlabel:<>] [ylabel:<>] [xrange:<>:<>] [yrange:<>:<>] [legend] | 
    #   <x> <y>[:<label>] [style:<>] [color:<>] | 
    #   [...]
    elif cmd=="plot":
        win_name = t_in[1]
        plt_name = t_in[2]
        loc = int(t_in[3])
        ax = windows[win_name]["fig"].add_subplot(loc)

        curline = None
        lines = []
        lastlines = []
        linecols = []
        legend = False
        autoscalex = True
        autoscaley = True

        # Get general/subplot settings
        spos = 4
        for o in t_in[4:]:
            osp = o.split(':')
            spos += 1
            if o.startswith("title:"):
                ax.set_title(osp[1].replace('__', ' '))
            elif o.startswith("xlabel:"):
                ax.set_xlabel(osp[1].replace('__', ' '))
            elif o.startswith("ylabel:"):
                ax.set_ylabel(osp[1].replace('__', ' '))
            elif o.startswith("xrange"):
                autoscalex = False
                ax.set_xbound(float(osp[1]), float(osp[2]))
                ax.set_xlim(float(osp[1]), float(osp[2]))
            elif o.startswith("yrange"):
                autoscaley = False
                ax.set_ybound(float(osp[1]), float(osp[2]))
                ax.set_ylim(float(osp[1]), float(osp[2]))
            elif o=="legend":
                legend = True
            # If start of lines
            elif o=="|":
                curline = 0
                break
                
        if curline is None:
            print("ERROR: no lines specified in subplot")
            continue

        # Get all line settings
        clen = len(t_in)
        while spos<clen:
            xsettings = t_in[spos].split(':')
            ysettings = t_in[spos+1].split(':')
            spos += 2

            xname = xsettings[0]
            if xname not in cols:
                cols[xname] = []

            yname = ysettings[0]
            ylabel = yname
            if yname not in cols:
                cols[yname] = []
            if len(ysettings)>1:
                ylabel = ysettings[1]

            # create (empty) line
            line, = ax.plot(cols[xname], cols[yname], label=ylabel.replace('__', ' '))
            if len(cols[xname])>0 and len(cols[yname])>0:
                lastline, = ax.plot([cols[xname][-1]], [cols[yname][-1]], 'kx')
            else:
                lastline, = ax.plot([0], [0], 'kx')
            lastline.set_color(line.get_color())
            lines.append(line)
            lastlines.append(lastline)
            linecols.append((cols[xname], cols[yname]))

            # get line settings for current line
            for o in t_in[spos:]:
                osp = o.split(':')
                spos += 1
                if o=='|':
                    curline += 1
                    break
                elif o.startswith("color:"):
                    line.set_color(osp[1])
                elif o.startswith("style"):
                    line.set_linestyle(osp[1])

        if legend:
            ax.legend()

        ax.grid(visible=True, which='both')

        # Save plot information
        Pd = {
            "ax" : ax,
            "lines" : lines,
            "lastlines" : lastlines,
            "linecols" : linecols,
            "autoscale" : (autoscalex, autoscaley),
        }
        windows[win_name]["plots"][plt_name] = Pd

    # Set incomming data format
    # format col1 col2 col3 ....
    elif cmd=="format":
        colformat = []
        for o in t_in[1:]:
            colformat.append(o)
            if o not in cols:
                cols[o] = []

    # Save incomming data
    # d d1 d2 d3 ...
    elif cmd=="d":
        i = 0
        for o in t_in[1:]:
            cols[colformat[i]].append(float(o))
            i += 1

    # Update graphs with new data
    # update
    elif cmd=="update":
        dataupdate()