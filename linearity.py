#!/usr/bin/env python

#make compatible 2.7 and 3
from __future__ import print_function
#get root
# from ROOT import TFile, TH1F, TDirectoryFile
import ROOT as root
#get the OS features
import os, sys, re, math

#fit a poisson distribution
def fitPoisson(hist):
    fit = root.TF1("fit","[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp(-([1]/[2])))/TMath::Gamma((x/[2])+1)", 1, 2500)
    fit.SetParameters(1,1,1)
    if hist.GetMean() > 0.51:
        status = int(hist.Fit("fit","RMQN"))
    else:
        status = 0
        # if status != 4000:
            # print(status)
            # hist.Draw()
            # raw_input("Press Enter to Continue...")
    return fit, status

#extract mean and sigma from 1D projections of # of Clusters histograms
def getParams(hist, ring):
    ringhist= hist.ProjectionY("Ring", ring+1, ring+1)
    # (fit,status)=fitPoisson(ringhist)
    # if status != 4000:
    mean = ringhist.GetMean()
        # print("Problem with the fit, using simple mean - fit status:",status)
    # else:
        # mean = fit.GetParameter(1)
    if mean == 0.5:
        mean = 0
        sigma = 0
    else:
        sigma = math.sqrt(mean)
        # sigma = ringhist.GetRMS()
    # if mean == 0.5:
        # mean = 0

    print("Ring",ring," wiht mean",mean,"RMS",sigma)
    return (mean,sigma)


# extract the mean, sigma and pileup from the folder in the rootfile - this is where the magic happens
def fillLinearityGraph(file,observable, graphs=[]):
    pileup = int(filter(str.isdigit, file))
    print("Found a root file for pileup", pileup, "in file", file, "Objects:",observable)

    rootfile = root.TFile.Open(file)
    rootfile.cd('BRIL_IT_Analysis/'+observable)

    #build the histogram names
    if observable == "Clusters":
        histname = "Number of clusters for Disk "
    elif observable == "2xCoincidences":
        histname = "Number of 2x Coincidences for Disk "
    elif observable == "3xCoincidences":
        histname = "Number of 3x Coincidences for Disk "

    #loop the disks
    for disk in range(1,5):
        histminusz = root.gDirectory.Get(histname +"-"+str(disk))
        histplusz = root.gDirectory.Get(histname+str(disk))
        # print(histminusz.GetName())
        # print(histplusz.GetName())
        #add plus and minus Z histograms
        histminusz.Add(histplusz)

        #now loop the rings
        for ring in range(5):
            (mean, sigma) = getParams(histminusz, ring)
            graphs[disk-1][ring].SetPoint(graphs[disk-1][ring].GetN(),pileup, mean)
            graphs[disk-1][ring].SetPointError(graphs[disk-1][ring].GetN()-1,0, sigma)

    rootfile.Close()
    return

def extrapolateLinearDumb(graph, basis=1):
    #or fit with TF1, get confidence interval 1Sigma
    #first, fit a linear fit with parameter 1 fixed to 0 (no intercept so it passes (0,0))
    pol1 = root.TF1("pol1",
                   "[0]*x",0,basis)
    # pol1.FixParameter(1,0)
    graph.Fit("pol1","R")
    par0 = pol1.GetParameter(0)
    extrapolated= root.TF1("extra","[0]*x",0,200)
    extrapolated.SetParameter(0,par0)
    extrapolated.SetLineColor(6)

    #now manually extrapolate PU1 min/max error to PU 200 and set TGraph Errrors
    x=root.Double(0)
    y=root.Double(0)
    for point in range(graph.GetN()):
        graph.GetPoint(point,x,y)
        if x == basis:
            Pu1error = graph.GetErrorY(point)
            slope = y/x
            slopehigh = (y+Pu1error)/x
            slopelow = (y-Pu1error)/x
            # print("Point 1 x:",x,"y:",y,"error:",Pu1error,"Slope:",slope,"high:",slopehigh,"low:",slopelow)
    #draw the extrapolation
    errors = root.TGraphAsymmErrors()
    errors.SetFillColor(6)
    errors.SetFillStyle(3003)
    #point (0,0)
    errors.SetPoint(0,0,0)
    errors.SetPointError(0,0,0,0,0)
    errors.SetPoint(1,200,200*slope)
    errors.SetPointError(1,0,0,200*slope-200*slopelow,200*slopehigh-200*slope)
    return(extrapolated,errors)

def extrapolateLinear(graph, basis=1):
    graph.Sort()
    #fit up to desired range, default is PU1
    pol1 = root.TF1("pol1",
                   "[0]*x",0,basis)
    graph.Fit("pol1","R")
    #draw the fit extrapolated to 200
    par0 = pol1.GetParameter(0)
    extrapolated = root.TF1("extra","[0]*x",0,200)
    extrapolated.SetParameter(0,par0)
    extrapolated.SetLineColor(6)

    #draw a TGRaphErrors with the 1sigma confidence interval
    errors = root.TGraphErrors()
    for point in range(graph.GetN()):
        errors.SetPoint(point,graph.GetX()[point],0)
        # print("x:",graph.GetX()[point],"point:",point)

    root.TVirtualFitter.GetFitter().GetConfidenceIntervals(errors,0.95)
    errors.SetFillColor(6)
    errors.SetFillStyle(3003)

    return(extrapolated,errors)

def relativeNonlinearity(graph, fit):
    deviation = root.TGraph()
    deviation.SetTitle("Deviation from Linearity;PU;Diff[%]")
    deviation.SetMarkerStyle(8)
    graph.Sort()
    x=graph.GetX()
    y=graph.GetY()

    for point in range(graph.GetN()):
        diff = y[point]-fit.Eval(x[point])
        expect = fit.Eval(x[point])
        if expect == 0:
            relative_diff = 0
        else:
            relative_diff = diff/expect
        deviation.SetPoint(point, x[point],relative_diff*100)
        print("x:",x[point],"y:",y[point],"eval y:",expect,"point:",point,"diff:",relative_diff*100,"%")

    return deviation

# def main():
disks,rings = 4,5
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
extrapolated = [[root.TF1() for j in range(rings)] for i in range(disks)]
errors = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]
graphs2x = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]
extrapolated2x = [[root.TF1() for j in range(rings)] for i in range(disks)]
errors2x = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]
graphs3x = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]
extrapolated3x = [[root.TF1() for j in range(rings)] for i in range(disks)]
errors3x = [[root.TGraphErrors() for j in range(rings)] for i in range(disks)]

# a TCanvas
c_clusters = root.TCanvas("Cluster Summary","Cluster Summary")
c_2x = root.TCanvas("2x Summary","2x Summary")
c_3x = root.TCanvas("3x Summary","3x Summary")
c_clusters.Divide(5,4)
c_2x.Divide(5,4)
c_3x.Divide(5,4)

#loop all root files
for file in files:
    if file.find(".root"):
        filename = path+file
        fillLinearityGraph(filename,"Clusters",graphs)
        fillLinearityGraph(filename, "2xCoincidences",graphs2x)
        fillLinearityGraph(filename, "3xCoincidences",graphs3x)


rootfile = root.TFile("Results.root","RECREATE")


index = 1
for i in range(disks):
    for j in range(rings):
        #do all the graphic stuff
        graphs[i][j].SetLineColor(1)
        graphs2x[i][j].SetLineColor(2)
        graphs3x[i][j].SetLineColor(4)
        graphs[i][j].SetTitle("Linearity Disk"+str(i+1)+"Ring"+str(j+1)+";Pileup;# of Clusters")
        graphs2x[i][j].SetTitle("Linearity Disk"+str(i+1)+"Ring"+str(j+1)+";Pileup;# of 2x Coincidences")
        graphs3x[i][j].SetTitle("Linearity Disk"+str(i+1)+"Ring"+str(j+1)+";Pileup;# of 3x Coincidences")

        # draw the actual graphs
        c_clusters.cd(index)
        graphs[i][j].Draw("ap")
        (extrapolated[i][j],errors[i][j]) = extrapolateLinear(graphs[i][j],10)
        errors[i][j].Draw("e3 same")
        extrapolated[i][j].Draw("same")

        # testcanvas = root.TCanvas("test","test")
        # testcanvas.cd()
        deviation = relativeNonlinearity(graphs[i][j], extrapolated[i][j])
        deviation.Write("Deviation Disk"+str(i+1)+"Ring"+str(j+1))
        # deviation.Draw("ap")
        # deviation.GetYaxis().SetRangeUser(-10,10)
        # deviation.Draw("ap")

        savecanvas = root.TCanvas("Clusters Disk"+str(i+1)+"Ring"+str(j+1),"Clusters Disk"+str(i+1)+"Ring"+str(j+1))
        savecanvas.cd()
        graphs[i][j].Draw("ap")
        # (extrapolated[i][j],errors[i][j]) = extrapolateLinear(graphs[i][j],10)
        errors[i][j].Draw("e3 same")
        extrapolated[i][j].Draw("same")
        savecanvas.Write("Clusters Disk"+str(i+1)+"Ring"+str(j+1))

        c_2x.cd(index)
        graphs2x[i][j].Draw("ap")
        (extrapolated2x[i][j],errors2x[i][j]) = extrapolateLinear(graphs2x[i][j],20)
        # errors2x[i][j].Draw("e3 same")
        extrapolated2x[i][j].Draw("same")

        # savecanvas = root.TCanvas("2x Disk"+str(i+1)+"Ring"+str(j+1),"2x Disk"+str(i+1)+"Ring"+str(j+1))
        savecanvas.cd()
        graphs2x[i][j].Draw("ap")
        # (extrapolated2x[i][j],errors[i][j]) = extrapolateLinear(graphs2x[i][j],20)
        # errors2x[i][j].Draw("e3 same")
        extrapolated2x[i][j].Draw("same")
        savecanvas.Write("2x Disk"+str(i+1)+"Ring"+str(j+1))

        c_3x.cd(index)
        graphs3x[i][j].Draw("ap")
        # (extrapolated3x[i][j],errors3x[i][j]) = extrapolateLinear(graphs3x[i][j],200)
        # errors2x[i][j].Draw("e3 same")
        # extrapolated2x[i][j].Draw("same")

        # savecanvas = root.TCanvas("3x Disk"+str(i+1)+"Ring"+str(j+1),"3x Disk"+str(i+1)+"Ring"+str(j+1))
        savecanvas.cd()
        graphs3x[i][j].Draw("ap")
        # # (extrapolated2x[i][j],errors[i][j]) = extrapolateLinear(graphs[i][j],10)
        # # errors2x[i][j].Draw("e3 same")
        # extrapolated3x[i][j].Draw("same")
        savecanvas.Write("3x Disk"+str(i+1)+"Ring"+str(j+1))

        #draw the extrapolation
        index = index+1

# c.SaveAs("linearity.pdf")
c_clusters.Write("SummarClusters")
c_2x.Write("Summar2x")
c_3x.Write("Summar3x")

rootfile.Close()

# if __name__ == '__main__':
        # main()
