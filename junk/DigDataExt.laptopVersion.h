// ==============================================================================
// Digitizer Data Extractor Class
// ==============================================================================
// Peter Szabo
// Created 02/09/2018
// Updated 31/12/2018
//
// Read a raw digitizer data file (.DIG) and save the data as a serial stream
// to a DireXeno Data file (.DXD).
//
// NOTES:
// 1. The output file is always appended with the data.
//
// 2. Data for one PMT is a sequence of numbers:
//
//    time tag          1               float
//    start index       1               float
//    samples           TIME_SAMPLES    float
//
//    Every (PMT_NUM + 4) of such sequences form an event.
//    The additional four sequences are the fast triggers TR0 & TR1,
//    each sampled along with two different groups of channels.
//    Event size in bytes is thus:
//    (PMT_NUM + 4) * (2 + TIME_SAMPLES) * sizeof(float)
//
// 3. All types of offline correction by CAEN are applied
//    (see "Data Correction" in CAEN V1742 user manual).
//
// 4. X742DecodeRoutines and X742CorrectionRoutines source files have a
//    slightly modified version for Direxeno, so we locally use:
//    X742DecodeRoutines.DX.c
//    X742DecodeRoutines.DX.h
//    X742CorrectionRoutines.DX.c
//    X742CorrectionRoutines.DX.h
//
// 4. Last event in each file is dropped, since it is often corrupt by a
//    user-abort (e.g., ctrl+c) while taking data.
//

#ifndef DIGDATAEXT_H
#define DIGDATAEXT_H

#include <string>
#include <iostream>
#include <fstream>
#include "CAENDigitizerType.h"
#include <vector>
#include "TH1F.h"
#include "TObject.h"
#include "TColor.h"
#include "TGraph.h"
#define MAX_CFG_STR 1000 // maximal length in chars of a single config file line

class DigDataExt
{
  public:
    DigDataExt(const std::string setup_file_name, const std::string dig_file_name, const std::string out_file_name);
  ~DigDataExt() { std::cout << "DigDataExt class Done." << std::endl; }//delete[] DataBuffer; std::cout << "DigDataExt class Done." << std::endl; }
  TString run_time_stamp;
  TString run_id;
  private:
    const std::string SetupFileName;
    const std::string DigFileName;
    const std::string OutFileName;
    
    char *DataBuffer;
    int DataBufferSize;
    int NumOfEvents;
    
    int ParseSetupFile(const std::string SetupFileName);
    int PMTNum = -999; // total number of PMTs
    int PMTChMap[MAX_X742_CHANNEL_SIZE * MAX_X742_GROUP_SIZE]; // digitizer channel (0..31) for each PMT (0..PMTNum-1)
  std::vector<std::string> PMTNameMap;
  std::vector<std::string> PMTTitle;
    uint32_t TimeSamples = -999; // samples in one waveform
    float TimeRes = -999; // samples per second
  int CorrMask = -999;
  std::string CorrFilesPath = "-999";

    // int const PMTChannel[PMT_NUM] = PMT_CH_MAPPING;
 };
void draw_Pannel_hist(TH1F *h[], TString name, int logy, int fits) ;
void draw_Pannel_graph(TGraph *g[], TString name) ;
void drawObjects(TObject** objects, TString name, TString title="Direxeno by channel", TString timestring="hh:mm:ss", int logy=0, int fits=0, int logz=0);
void draw2Objects(TObject** objects1, TObject** objects2,TString name, TString title="Direxeno by channel",TString timestring="hh:mm:ss",int logy=0, int fits=0, int logz=0);

void CombineMeanAndRMS(Double_t& combinedMean, Double_t& combinedRMS,const std::vector<Double_t>& meanValues,const std::vector<Double_t>& rmsValues, int numMeasurements);
#endif



// EOF
