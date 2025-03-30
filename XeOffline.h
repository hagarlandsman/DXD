#ifndef __X742_XEOFFLINE_H
#define __X742_XEOFFLINE_H
#include "X742DecodeRoutines.h"

//#include "WaveDump.h"
#include <stdio.h>
#include <stdlib.h>
#include <TCanvas.h>
//#include <CAENDigitizer.h>
#include <TTree.h>
//#include "WDconfig.h"
#include <TFile.h>
#include <TGraph.h>
//static CAEN_DGTZ_IRQMode_t INTERRUPT_MODE = CAEN_DGTZ_IRQ_MODE_ROAK;
using namespace std;

static long get_time();
typedef enum
  {
ERR_NONE= 0,
  ERR_CONF_FILE_NOT_FOUND,
  ERR_DGZ_OPEN,
  ERR_BOARD_INFO_READ,
  ERR_INVALID_BOARD_TYPE,
  ERR_DGZ_PROGRAM,
  ERR_MALLOC,
  ERR_RESTART,
  ERR_INTERRUPT,
  ERR_READOUT,
  ERR_EVENT_BUILD,
  ERR_HISTO_MALLOC,
  ERR_UNHANDLED_BOARD,
  ERR_OUTFILE_WRITE,
  ERR_OVERTEMP,

  ERR_DUMMY_LAST,
  } ERROR_CODES;
typedef struct {
      Float_t area,min,max,locmin,locmax,amp0,mean0,rms0,chi0,amp,mean,rms,chi;
   } CHANNEL;

class XeOffline : public TObject  {

public:

  XeOffline( const char * , string dir="./");

//  XeOffline( TString );
  ~XeOffline(){};
  XeEvent * decodeEvent(int N);
  //CAEN_DGTZ_X742_EVENT_t * decodeEvent(int N);
  void saveOne(int N);
  void loadCorrectionTables(string dir="./") ;

  TGraph ** toGraph(XeEvent *Event);
  TGraph ** toGraph(int N);
  long int getTusec(int N);
  long int  getTgroup(int N, int group);
  FILE *fbin;
  long int T0usec;
  int groupT;
  int eventT;
  long int groupTns[4];
  float Tstep;
  const char *filename;
  long int lsize;
  uint32_t eventNumber;
  uint32_t numEvents;
  char *buff;
  void plotAll(float Tstamp) ;
  //CAEN_DGTZ_UINT16_EVENT_t        *Event;
  CAEN_DGTZ_BoardInfo_t       BoardInfo;
  //  CAEN_DGTZ_BoardInfo_t       BoardInfo2;
  CAEN_DGTZ_EventInfo_t       *EventInfo;
  void setSaveWF(int val) {if (val==0 || val==1) saveWF=1; else printf ("error, value can be 0 or 1"); }
  TTree *tree;
  int Nc;
  int getNpmts(){ return Npmts;}
  uint32_t getN(){return numEvents;}
  void CorrectionsMode(int m) { 
    if (m==0) {DO_CORRECTION=0; printf ("disabling corrections \n");} 
    else if (m==1) {DO_CORRECTION=1; if (CORRECTION_LOADED==0) printf ("Caution, no correction tables loaded !\n"); printf ("enabling corrections \n");}
  };
private:
  int Npmts;
  TGraph **gV;
  CAEN_DGTZ_DRS4Correction_t* CorrTable[MAX_X742_GROUP_SIZE];

  int DO_CORRECTION;
  int CORRECTION_LOADED;
  int pmtCh[20]= {0,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,1,2,3};//;{1,2,4,6,7,8,10,12,13,15,16,18,20,22,23,24,26,28,30,31};
  long ptime;
  CHANNEL ch[32]={  {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0},
      {0,0,0,0,0,0,0,0,0,0}};

  TH1F **hWF; //[20];
  TH1 **hWFb;
  //TH1F **h0;
  //TH1F **h;
  TGraph **g;
  TGraph **gPeaks;
  void peaks(TH1F *h, TH1 *hb,TGraph *gP);
    int saveWF;
    //  float Ts;
  int DataSummary();
  int fcount;
  int tcount;
    uint64_t CurrentTime, PrevRateTime, ElapsedTime, StatElapsedTime, prevStatTime;
    //    CAEN_DGTZ_UINT16_EVENT_t    *Event16=NULL; /* generic event struct with 16 bit data (10, 12, 14 and 16 bit digitizers */
//    TCanvas *c1;
 //   TCanvas *c2;
  CAEN_DGTZ_DRS4Correction_t X742Tables[MAX_X742_GROUP_SIZE];
    uint32_t AllocatedSize, BufferSize, NumEvents;
    char *buffer;
    int Ngroups;
  int ReloadCfgStatus;
  void GoToNextEnabledGroup();
  void reload();
  CAEN_DGTZ_ErrorCode ret;
  int  handle;
  ERROR_CODES ErrCode;
  char ConfigFileName[100];
  int programDigitizer();
  int WriteRegisterBitmask(uint32_t address, uint32_t data, uint32_t mask);
  int plotBoth;
  int plot1;
  int plot2;
  int Nsave;
  TGraph *gDummy;
  TH1F *hWFDummy;
  TFile *hfile;
  uint64_t timeLastPlot;
//  ClassDef(XeOffline,1)
};


#endif
