#include <sys/time.h>
#include <ctime>
//#include "WDplot.h"
//#include <iostream>
//#include "WDconfig.h"
#include "WaveDump.h"
#include <stdio.h>
#include <stdlib.h>
#include <bitset>
#include "X742DecodeRoutines.h"

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
//#include <ncurses.h>
#include <TTree.h>
#include <TFile.h>
#include <TSpectrum.h>
#include <TVirtualFitter.h>
#include <TPolyMarker.h>
//extern "C" int ParseConfigFile(FILE*, WaveDumpConfig_t*);// void f(int);

using namespace std;

XeOffline::XeOffline(const char *filename_,string dir_){
   printf ("openning %s \n",filename_);
  if ((fbin = fopen(filename_, "rb")) == NULL) {
    printf ("Problem reading binary file %s ",filename_);
    lsize=0;
    numEvents=0;
  }
  DO_CORRECTION=1;
  CORRECTION_LOADED=0;
filename=filename_;
  fseek (fbin , 0 , SEEK_END);
  lsize = ftell(fbin);
  rewind (fbin);
  buff = new char [lsize];
  fread(buff,sizeof(char),lsize,fbin);
  uint32_t numEvents_;
  loadCorrectionTables(dir_) ;
  GetNumEvents(buff, lsize, &numEvents_) ;
  numEvents=numEvents_;
  printf ("reading binary file %s with %d events\n ",filename,numEvents);
  Npmts=20;
  gV = new TGraph *[32];
  for (int i=0; i<Npmts; i++) gV[i]=new TGraph();


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
  // New part:
  int len[4]; // 12 bits, size, 1024 samples is 0xC00
   int tr[4];  // 1 bit, TR signal is present or not
   int freq[4]; // 2 bits, 00=5Gs/s, 01=2.5Gs/s, 10=1Gs/s, 11=not used
   int cell[4]; // 10 bits, start cell index of DRS4 SCA
   int trigger_time_tag[4]; // maybe 30 bits
 
   int adc[32][1024];
   int adc_tr[4][1024];
   
   bool adc_overflow[32];
   bool adc_tr_overflow[4]; 
  const uint32_t *data = (const uint32_t*)(EventPtr);
  /*  printf("Header:\n");
      printf("  total event size: 0x%08x (%d)\n", data[0], data[0]&0x0FFFFFFF);
      printf("  board id, pattern, gr mask: 0x%08x\n", data[1]);
      printf("  event counter: 0x%08x (%d)\n", data[2], data[2]);
      printf("  event time tag: 0x%08x (%d)\n", data[3], data[3]);
  */
// header word 1
   int board_id = (data[1]>>27)&0x1F;
   int pattern  = (data[1]>>8)&0x3FFF;
   int group_mask =  data[1] & 0xF;
   // header word 2
   int event_counter = data[2] & 0x3FFFFF;
   // header word 3
   int event_time_tag = data[3];
   const uint32_t *g = data + 4;
   for (int i=0; i<4; i++) {
      
      if (((1<<i)&group_mask)==0)
         continue;

      len[i]  = g[0] & 0xfff;
      tr[i]   = (g[0]>>12)&1;
      freq[i] = (g[0]>>16)&3;
      cell[i] = (g[0]>>20)&0x3ff;

      g += 1;
      // g points to the data
      
      //for (int k=0; k<10; k++)
      //	printf("  adc data[k]: 0x%08x\n", g[k]);
      
      int k=0;
      
      const uint8_t* p = (const uint8_t*)g;
      int x = 0;
      for (int s=0; s<1024; s++)
         for (int a=0; a<8; a++) {
            int v = 0;
            if (x==0) {
               v = (p[0]) | ((p[1]&0xF)<<8);
               p += 1;
               x = 1;
            } else {
               v = ((p[0]&0xF0)>>4) | ((p[1]&0xFF)<<4);
               p += 2;
               x = 0;
            }
            
            //printf("group %d, channel %d, sample %d: value %6d (0x%03x)\n", i, a, s, v, v);
            
            adc[i*8+a][s] = v;
            
            if (v == 0)
               adc_overflow[i*8+a] = true;
            
            k++;
            //if (k > 10)
            //abort();
         }
      
      g += len[i];
      
      if (tr[i]) {
         int trlen = len[i]/8;
         
         const uint8_t* p = (const uint8_t*)g;
         int x = 0;
         for (int s=0; s<1024; s++) {
            int v = 0;
            if (x==0) {
               v = (p[0]) | ((p[1]&0xF)<<8);
               p += 1;
               x = 1;
            } else {
               v = ((p[0]&0xF0)>>4) | ((p[1]&0xFF)<<4);
               p += 2;
               x = 0;
            }
	    
            //printf("group %d, channel %d, sample %d: value %6d (0x%03x)\n", i, a, s, v, v);
	    
            adc_tr[i][s] = v;
            
            if (v == 0)
               adc_tr_overflow[i] = true;
	    
            k++;
            //if (k > 10)
            //abort();
         }
         
         g += trlen;
      }
         // g points to the time tag
      if (0) {
         printf("  group trigger time tag: 0x%08x\n", g[0]);
      }

      trigger_time_tag[i] = g[0];

      g += 1;
      // g point s to the next group
   }
   //   printf ("trigger time tag=%ld %ld \n",trigger_time_tag[0], event_time_tag);

   //////////////////// end of new part

  // printf ("got %d \n",c);
  c = X742_DecodeEvent(EventPtr, (void**) &Event);
   Event->groupT=trigger_time_tag[0];
   Event->eventT=event_time_tag;

  int CorrectionLevelMask=1+2+4;
  bitset<3> CorrMask;
  CorrMask.set();
  int    CorrInt = CorrMask.to_ulong();
  //printf ("%d %d \n",CorrInt,CorrMask);
  if (DO_CORRECTION && CORRECTION_LOADED)
  for (int j = 0; j < MAX_X742_GROUP_SIZE; j++)
    {
     // printf ("correcting freq= %d, correction \n",Event->Freq);
      ApplyDataCorrection(CorrTable[j], (CAEN_DGTZ_DRS4Frequency_t)Event->Freq, CorrInt, &Event->DataGroup[j]);
    }
  /*
 00 = 5GS
 01 = 2.5GSs
 10 = 1GS
 11 = not used
  */
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
 // delete g;
}

void XeOffline::loadCorrectionTables(string dir) {
  CORRECTION_LOADED=1;
  for (int i = 0; i < MAX_X742_GROUP_SIZE; i++)
  {
    // CorrTable[i] = (CAEN_DGTZ_DRS4Correction_t*) malloc(sizeof(CAEN_DGTZ_DRS4Correction_t));
    CorrTable[i] = new CAEN_DGTZ_DRS4Correction_t;
    string s = dir + "X742Table_gr" + to_string(i);
    cout << s << "_*.txt   (* = cell, nsample, time)" << endl;
    if (0 > LoadCorrectionTable((char*)s.c_str(), CorrTable[i])) { cerr << "*** Correction table " << s << " read error. ***" << endl << endl; exit(1);  CORRECTION_LOADED=0;}
  }
 


}
long int XeOffline::getTusec(int N){
   XeEvent * Event=decodeEvent(N);
   T0usec=Event->T0/50;
   return T0usec;
}

long int XeOffline::getTgroup(int N, int group){
   XeEvent * Event=decodeEvent(N);
   for (int g=0; g<4; g++) groupTns[g]=Event->groupTtag[g]*8.5;
   return Event->groupTtag[group];

}

  void printTgroup(int N);
  long int getTgroup(int N, int group);

TGraph **XeOffline::toGraph( int N){
   XeEvent * Event=decodeEvent(N);
   T0usec=Event->T0/50;
   groupT=Event->groupT;
   eventT=Event->eventT;

   for (int g=0; g<4; g++) groupTns[g]=Event->groupTtag[g]*8.5;
   // for (int g=0; g<4; g++) printf ("%ld   ",groupTns[g]);
   eventNumber = Event->eventNumber;

 // fprintf (fplot0,"# Tstamp= %f usec, \n",Event->T0/50.);///50);
   return (toGraph(Event));
}
TGraph **XeOffline::toGraph(XeEvent *Event){
//  TGraph **g = new TGraph *[32];

//  int color=1;
  int Ngot=0;
  int size0;
  //float Ts;
  float t;
  for (int iPMT=0; iPMT<Npmts; iPMT++) {
    gV[iPMT]->Set(0);
   // g[iPMT]=new TGraph();
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
	  gV[iPMT]->SetPoint(gV[iPMT]->GetN(),t,Event->DataGroup[group].DataChannel[cha][i]);
	}
      }
    }
  }
  return gV;
}
