#include <sys/time.h>
#include <ctime>
//#include "WDplot.h"
//#include <iostream>
//#include "WDconfig.h"
#include <stdio.h>
#include "TApplication.h"

#include <stdlib.h>
#include <iostream>
#include <stdint.h>
//#include <ncurses.h>
//#include "fun.cpp"
#include <stdio.h>
#include <CAENDigitizer.h>
#include "X742DecodeRoutines.h"
#include "X742CorrectionRoutines.h"
#define GNUPLOT_COMMAND  "gnuplot"
#define PLOT_DATA_FILE   "PlotData.txt"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TMath.h"

//#include <ncurses.h>
#include <TTree.h>
#include <TFile.h>
#include <TSpectrum.h>
#include <TVirtualFitter.h>
#include <TPolyMarker.h>
#include "XeOffline.h"
using namespace std;
int main(int argc, char **argv) {
  TApplication theApp("Analysis", &argc, argv);
  char *fileName="waves_0001.dat";
  if (argc==2) fileName=argv[1];
  //fileName="out_0001.dat";
 // XeOffline *xef=new XeOffline("/data/Gera/190109_pulser/out_0001.dat","/data/Gera/190109_pulser/");
  //  XeOffline *xef=new XeOffline("/data/out_28112020b/out_0001.dat");
  //  XeOffline *xef=new XeOffline("/data/301120/NG1/out_0004.dat");
  //  XeOffline *xef=new XeOffline("/home/access/Direxdaq/XeDAQ/out_07122020_pulserOnQuite/out_0002.dat");
//  XeOffline *xef=new XeOffline("/home/access/Direxdaq/XeDAQ/out/out_0001.dat");
// XeOffline *xef=new XeOffline(Form("/data/out_11122020_NG_NOVETO_5Hz_1ms/%s",fileName)); 
  XeOffline *xef=new XeOffline(Form("/data/out_VETO_NG_QUITESPHERE/%s",fileName)); 

  // XeOffline *xef=new XeOffline(Form("/data/021220/NG4/%s",fileName)); 


 // XeOffline *xef=new XeOffline(Form("/data/out_20201213_NG_VETO_QUITE_5Hz_1ms/%s",fileName));



  TH1F *hh[32]; for (int p=0;p<32;p++) hh[p]=new TH1F(Form("h%d",p),"h",5000,0,4096);
  TGraph*gr[32]; for (int p=0;p<32;p++) gr[p]=new TGraph(); 
  uint32_t numEvents = xef->numEvents;
  printf (" Got %d events \n",numEvents); 
  TGraph **g=xef->toGraph(10);
  TGraph*gm[32]; for (int p=0;p<32;p++) gm[p]=new TGraph(); 
  gm[2]->Draw("A*");
 TCanvas *c2=new TCanvas("c2","c2",800,800);  

  TCanvas *c1=new TCanvas("c1","c1",800,800);  
  c1->Divide(2,2);
  TGraph *gdummy=new TGraph();
  gdummy->SetPoint(0,0,0);
  gdummy->SetPoint(1,256,4000);
  for (int i=1; i<=4; i++) {c1->cd(i); gdummy->Draw("Ap");}
  long int pt=0;
  int counter=0;
  for (int e=0;e<numEvents; e++) {
    // if (e==1567 || e==1566 || e==1568 || e==3590 || e==3591 ) continue;
 // printf ("===============================\n");

    g=xef->toGraph(e); 

  //printf ("usec=%d group0=%d \n",xef->T0usec,xef->groupTns[0]);

    if (e%100000==0) {
    c1->cd(1); g[0]->Draw("*");
    c1->cd(2); g[1]->Draw("*");
    c1->cd(3); g[2]->Draw("*");
    c1->cd(4); g[3]->Draw("*");    c1->Update();
    }

   long int thisT=xef->groupTns[0]/1000.;
   long int  dt=thisT-pt;
   if (pt==0) dt=0;
   long int roundTime=8.5*pow(2,30)/1000.;

   if (dt<0) {//printf ("switching dt from %ld to %ld \n\n",dt,dt+8.5/1000.*pow(2,30)); 
     dt=dt+roundTime;counter++;}
  
   // printf ("%d %ld %ld %ld %f %f %ld ",e,xef->T0usec,thisT,+thisT+roundTime*counter,g[0]->GetMean(2), g[0]->GetMean(2),dt);    
      //  printf ("%d %f \n",i,g[i]->GetMean(2));
      if (g[0]->GetMean(2)<1000) 
	{ printf ("times  %d %ld %ld %ld %ld ",e,xef->groupT, xef->eventT,xef->T0usec,xef->groupTns[0]/8.5);	
	  printf ("%ld \t  mean= %f  <======= \n",thisT, g[0]->GetMean(2));
	}
      else 
	{ 
	  printf ("times  %d %ld %ld %ld %ld ",e,xef->groupT, xef->eventT,xef->T0usec,xef->groupTns[0]/8.5);
	  printf ("%ld \t mean=  %f  \n",thisT, g[0]->GetMean(2));
	}

      //      if (pt!=0 && pt>xef->groupTns[0]) counter++;
      
      pt=thisT;
    /*    for (int p=0; p<20;p++) {
      //for (int j=0; j<g[p]->GetN()-10; j++) hh[p]->Fill(g[p]->GetY()[j]); 
      double_t median=4096-TMath::Median(g[p]->GetN()-10,g[p]->GetY());
      double_t rms=TMath::RMS(g[p]->GetN()-10,g[p]->GetY());
      double_t median20=4096-TMath::Median(20,g[p]->GetY());
      double_t RMS20=TMath::RMS(20,g[p]->GetY());
      double_t xMax = TMath::LocMin(g[p]->GetN()-10,g[p]->GetY());		
      printf ("%d pmt %d, median=%f \t rms=%f \t median20=%f,\t rms20=%f,\t xmax=%f \n",e,p,median,rms,median20,RMS20,xMax); 
      hh[p]->SetTitle(Form("pmt=%d, e=%d",p,e)); 
      g[p]->SetTitle(Form("pmt %d, e=%d "));

//      gm[p]->SetPoint(gm[p]->GetN(),xef->getTusec(e),TMath::Median(g[p]->GetN(),g[p]->GetY())); 
      //gr[p]->SetPoint(gr[p]->GetN(),xef->getTusec(e),TMath::RMS(g[p]->GetN(),g[p]->GetY()));if (e%200==0) gPad->Update(); 
      gm[p]->SetTitle(Form("e=%d, pmt=%d ",e,p));
      gr[p]->SetTitle(Form("e=%d, pmt=%d ",e,p));
      g[p]->Draw("A*"); gPad->Update();
      char *temp=new char[1];    gets(temp);
       
      }*/
  }

  // delete Event;
  //  for (int i=0; i<numEvents; i++) { printf ("%d \n",i);
  // xef->saveOne(i);
  // }


  
  //  TGraph **g=xef->toGraph(10);
  //  g[0]->Draw("Al");
 
 

  ////      theApp.Run();




}



//   FILE *ffbout;
//   char fbname[100];
//   int fcounter=1;
//   CAEN_DGTZ_X742_EVENT_t       *Event; 
//   int handle = 0;
  


//   Event = NULL;
//   char * buffer;
//   size_t result;
// uint32_t BufferSize, NumEvents,line;

//   sprintf(fbname, "waves_%04d.dat", fcounter);
//     if ((ffbout = fopen(fbname, "rb")) == NULL) {
//       printf ("Problem reading binary file %s ",fbname);
//     return -1;
//   }
//   fseek (ffbout , 0 , SEEK_END);
//   long int lsize = ftell(ffbout);
//   rewind (ffbout);
//   printf ("reading binary file %s\n ",fbname);
//   long int currentLocation=0;
//   long int nextLocation=0;
//   int Nevents=0;
// CAEN_DGTZ_EventInfo_t       EventInfo;
// char *EventPtr = NULL;
//  char *buff = new char [lsize];
//  fread(buff,sizeof(char),lsize,ffbout);  
//  uint32_t numEvents;
//  GetNumEvents(buff, lsize, &numEvents) ;
//  printf ("n= %d new \n",numEvents);
//  int c=GetEventPtr(buff, lsize, 10, &EventPtr);
//  printf ("got %d \n",c);
//  // c = X742_DecodeEvent(EventPtr, (void**) &Event);
 
//  printf ("got %d \n",c);
// //  int	  ret = CAEN_DGTZ_DecodeEvent(0, ffbout, (void**)&Event);

//   while (currentLocation<lsize) {


//     fread(&line,sizeof(uint32_t),1,ffbout);    
//     int Nbytes=4*(line&0x0FFFFFFF);


//     //    int ret=X742_DecodeEvent(buff,(void**)&Event);
   
//     // int ret = CAEN_DGTZ_GetEventInfo(0, buff, Nbytes/4, 1, &EventInfo, &EventPtr);
//     //    printf ("ret= %d \n",ret);
//     // ret = CAEN_DGTZ_DecodeEvent(0, EventPtr, (void**)&Event);
//     //printf ("ret= %d \n",ret);

//     //  std::ofstream stream("yourFile", std::ios::binary);
//   nextLocation=currentLocation+Nbytes;
//     printf ("num %d = num bytes=%d \n",line&0x0FFFFFFF, Nbytes);
//     printf ("control %x = num \n",(line&0xF0000000)>>4*7);
//     printf ("full %x = num \n",line);
//     if ((line&0xF0000000)>>4*7!=0xa) {
//       printf ("problem.....");
//       return (-1);
//     }
//     Nevents++;
//     fseek ( ffbout , nextLocation , SEEK_SET );
  
//     currentLocation=ftell(ffbout);

//     printf ("   now at %ld \n",  ftell(ffbout));
//     printf (" moving to byte %ld to location %ld from %ld \n", Nbytes,currentLocation,lsize);
//   }
//   printf ("read %d Nevents \n",Nevents);
//     return 1;
// }

 
