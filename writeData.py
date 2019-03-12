#!/usr/bin/env python
""" This file is part of libLiFFT.

 libLiFFT is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 libLiFFT is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with libLiFFT. If not, see <www.gnu.org/licenses/>.
"""
 
from matplotlib import use as mplUse
mplUse('Agg')
from matplotlib import cm
from matplotlib import pyplot as plt
from matplotlib.colors import LogNorm
import numpy as np
import sys
import argparse

delta_x = 1.
delta_y = 1.

def drawGraphic(data, figure, xLabel = None, yLabel = None, title = None, logPlot = True, extent = None):

    if logPlot:
        norm = LogNorm()
    else:
        norm = None
        
    subplot = figure.add_subplot(111)
    pic = subplot.imshow(data, norm=norm, aspect="auto", cmap=cm.nipy_spectral, extent=extent)

    if xLabel:
        plt.xlabel(xLabel, fontsize=20)
    if yLabel:
        plt.ylabel(yLabel, fontsize=20)

    plt.xticks(size=16)
    plt.yticks(size=16)

    if title:
        plt.title(title, fontsize=24)
    CB = figure.colorbar(pic)
    for t in CB.ax.get_yticklabels():
        t.set_fontsize(20)
            
def writeSpectral(data, outFp):
  Nx = len(data[0])
  Ny = len(data)
  kx_max = np.pi * (Nx-1)/(Nx*delta_x)
  ky_max = np.pi * (Ny-1)/(Nx*delta_y)
  sizeX = 1024*20/Nx
  sizeY = 1024*16/Ny
  FIG = plt.figure(0, figsize=(sizeX, sizeY))
  drawGraphic(data,
               FIG,
               "$k_x$",
               "$k_y$",
               "Spectral colored data (arbitrary units)",
               True,
               (-kx_max, kx_max, -ky_max, ky_max))
  FIG.savefig(outFp, format='pdf')
  plt.close(FIG)
  
def writeNormal(data, outFp):
  Nx = len(data[0])
  Ny = len(data)
  sizeX = 1024*20/Nx
  sizeY = 1024*16/Ny
  FIG = plt.figure(0, figsize=(sizeX, sizeY))
  drawGraphic(data,
               FIG,
               r"xPos",
               r"yPos",
               "Spectral colored data (arbitrary units)")
  FIG.savefig(outFp, format='pdf')
  plt.close(FIG)

def loadAndWriteData(inFp, outFp, isSpectral):
  data = np.loadtxt(inFp, dtype='float32')
  if isSpectral:
    writeSpectral(data, outFp)
  else:
    writeNormal(data, outFp)

  
def main(name, argv):
  parser = argparse.ArgumentParser(description="Create PDF image from text input", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('-i', '--ifile', required=True, help="Path to inputfile")
  parser.add_argument('-o', '--ofile', default="output.pdf", help="Path to outputfile")
  parser.add_argument('-s', '--isSpectral', action="store_true", help="Use spectral scaled output")
  options = parser.parse_args(argv)
  
  print('Loading "' + options.ifile + '" to "' + options.ofile + '"')
  loadAndWriteData(options.ifile, options.ofile, options.isSpectral)
       
if __name__ == "__main__":
   main(sys.argv[0], sys.argv[1:])

