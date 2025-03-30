#include <sys/time.h>
#include <ctime>
//#include "WDplot.h"
//#include <iostream>
#include "WDconfig.h"
#include "WaveDump.h"
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
//#include "fun.cpp"
#include <stdio.h>
#include <CAENDigitizer.h>
#include "X742DecodeRoutines.h"
#include "X742CorrectionRoutines.h"
#include "XeOffline.h"
//#define GNUPLOT_COMMAND  "gnuplot"
//#define PLOT_DATA_FILE   "PlotData.txt"
#include "TCanvas.h"
#include "TGraph.h"
#include <ncurses.h>
#include <TTree.h>
#include <TFile.h>
#include <TSpectrum.h>
#include <TVirtualFitter.h>
#include <TPolyMarker.h>
//extern "C" int ParseConfigFile(FILE*, WaveDumpConfig_t*);// void f(int);

using namespace std;

ClassImp(XeOffline)
XeOffline::XeOffline(char *filename_){
  if ((fbin = fopen(filename_, "rb")) == NULL) {
    printf ("Problem reading binary file %s ",filename_);
    lsize=0;
    numEvents=0;
  }
  filename=filename_;
  fseek (fbin , 0 , SEEK_END);
  lsize = ftell(fbin);
  rewind (fbin);
  buff = new char [lsize];
  fread(buff,sizeof(char),lsize,fbin);
  GetNumEvents(buff, lsize, &numEvents) ;
  printf ("reading binary file %s with %d events\n ",filename,numEvents);

}
XeEvent * XeOffline::decodeEvent(int N){
  //CAEN_DGTZ_X742_EVENT_t       *Event=NULL;
  XeEvent  *Event=NULL;
  //printf (" Num= %d \%d \n",N,numEvents);
  if (N<0 || N>numEvents) {
    printf ("illegal event number should be 0....%d \n",numEvents);
    return (Event);
  }

  char *EventPtr = NULL;

  int c=GetEventPtr(buff, lsize, N, &EventPtr);
  // printf ("got %d \n",c);
  c = X742_DecodeEvent(EventPtr, (void**) &Event);

  //  printf ("got %d  %f ns\n",c,Event->Tstep);
  //for (int i=0;i<4; i++) {  printf ("   grpup %d,   %ld \n",i,Event->DataGroup[i].TriggerTimeTag);}

  return Event;
}
void XeOffline::saveOne(int N) {
  XeEvent * Event=decodeEvent(N);
  //  TGraph **g=toGraph(Event);
  Float_t Ts = Event->Tstep;
  //printf ("Ts= %f \n",Ts);
  char fname[100];
  //    int size=Event->DataGroup[0].ChSize[0];
  sprintf(fname, "%s_%d_%d.txt",filename,N,Event->eventNumber);
  FILE *fplot0= fopen(fname, "w");
  fprintf (fplot0,"# Tstamp= %f usec, \n",Event->T0/50.);///50);
  TGraph **g=toGraph(Event);
  for (int i=0; i<g[0]->GetN(); i++){

    fprintf (fplot0,"%0.2f",g[0]->GetX()[i]);
    for (int ic=0;ic<Npmts;ic++) { if (g[ic]->GetN()>ic) fprintf (fplot0,"\t%f", g[ic]->GetY()[i]); else  fprintf (fplot0,"\t");}
    fprintf (fplot0,"\n");
  }
}

TGraph **XeOffline::toGraph( int N){
   XeEvent * Event=decodeEvent(N);
   return (toGraph(Event));
}
TGraph **XeOffline::toGraph(XeEvent *Event){
  TGraph **g = new TGraph *[32];

  int color=1;
  int Ngot=0;
  int size0;
  //float Ts;
  float t;
  for (int iPMT=0; iPMT<Npmts; iPMT++) {
    g[iPMT]=new TGraph();
    int group=int(pmtCh[iPMT]/8);
    int cha=pmtCh[iPMT]%8;
    float Ts=Event->Tstep;
    if (Event->GrPresent[group]) {
      int size=Event->DataGroup[group].ChSize[cha];
      if (size <= 0) continue;
      else {
	size0=size;
	Ngot++;
	for (int i=0; i<size; i++) {
	  if (Ts==0) t=i;
	  else t=i*Ts;
	  g[iPMT]->SetPoint(g[iPMT]->GetN(),t,Event->DataGroup[group].DataChannel[cha][i]);
	}
      }
    }
  }
  return g;
}
