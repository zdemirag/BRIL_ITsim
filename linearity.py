#!/usr/bin/env python

#make compatible 2.7 and 3
from __future__ import print_function
#get root
# from ROOT import TFile, TH1F, TDirectoryFile
import ROOT as root
#get the OS features
import os, sys, re

def getParams(hist, ring):
    ringhist= hist.ProjectionY("Ring", ring+1, ring+1)
    mean = ringhist.GetMean()
    if mean == 0.5:
        mean = 0
    sigma = ringhist.GetRMS()
    print("Ring",ring," wiht mean",mean,"RMS",sigma)
    return (mean,sigma)

def fillLinearityGraph(file,observable, graphs=[]):
        pileup = int(filter(str.isdigit, file))
        print("Found a root file for pileup", pileup, "in file", file, "Objects:",observable)

        rootfile = root.TFile.Open(file)
        rootfile.cd('BRIL_IT_Analysis/'+observable)

        #now list the keys in the file
        for key in root.gDirectory.GetListOfKeys():
            histname = key.ReadObj().GetName()
            diskarr = [int(d) for d in re.findall(r'-?\d+', histname)]
            if diskarr:
               if observable == "Clusters":
                   disk=diskarr[0]
               else:
                   disk=diskarr[1]
            else:
                continue

            print("Found histogram for Disk", disk)
            hist = key.ReadObj()
            for ring in range(5):
               (mean, sigma) = getParams(hist, ring)
               if disk < 0:
                   thedisk = disk+4
               else:
                   thedisk = disk+3

               graphs[thedisk][ring].SetPoint(graphs[thedisk][ring].GetN(),pileup, mean)
               graphs[thedisk][ring].SetPointError(graphs[thedisk][ring].GetN()-1,0, sigma)


# def main():
disks,rings = 8,5
args = len(sys.argv)
if args>1:
    path = sys.argv[1]
    print(path)
else:
    path = "/afs/cern.ch/user/g/gauzinge/ITsim/mySummaryPlots/"

#now get the files in the path
files = os.listdir(path)
files.sort()

# a 2D array of TGraph Errors
#first index is disk and second is ring
graphs = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]
graphs2x = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]
graphs3x = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]

# a TCanvas
c = root.TCanvas("Summary","Summary")
c.Divide(8,5)

#loop all root files
for file in files:
    if file.find(".root"):
        filename = path+file
        fillLinearityGraph(filename,"Clusters",graphs)
        fillLinearityGraph(filename, "2xCoincidences",graphs2x)
        fillLinearityGraph(filename, "3xCoincidences",graphs3x)

index = 1
for i in range(disks):
    for j in range(rings):
        c.cd(index)
        graphs[i][j].SetLineColor(1)
        graphs2x[i][j].SetLineColor(2)
        graphs3x[i][j].SetLineColor(4)
        graphs[i][j].SetTitle("Linearity;Pileup;# of Clusters")
        graphs[i][j].Draw("ap")
        graphs2x[i][j].Draw("p same")
        graphs3x[i][j].Draw("p same")
        index = index+1


# if __name__ == '__main__':
        # main()
