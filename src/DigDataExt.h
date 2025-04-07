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
#include <ctime>

#define MAX_CFG_STR 1707 // maximal length in chars of a single config file line
# define FIXED_STRING_LEN 32
//#define MAX_CFG_STR 1000 // maximal length in chars of a single config file line

class DigDataExt
{
  public:
  DigDataExt(const std::string CorrFilesPath, const std::string setup_file_name, const std::string dig_file_name, const std::string out_file_name, int fast=1);
  void DigDataExtwROOT(const std::string CorrFilesPath, const std::string setup_file_name, const std::string dig_file_name, const std::string out_file_name, int fast=1);
  //~DigDataExt() { std::cout << "DigDataExt class Done." << std::endl; }//delete[] DataBuffer; std::cout << "DigDataExt class Done." << std::endl; }
  std::string run_time_stamp;
  std::string run_id;
  void write_tag(std::ostream& fout, const char* tag) ;
  void WriteChannelToMeta(std::fstream& out,
    int e, long TimeTag, float Tsamp, float StartIndex,
    int g, int c, int ch, const std::string& name,
    int pmt_ch, const std::vector<float>& waveform);

  private:
    const std::string SetupFileName;
    const std::string DigFileName;
    const std::string OutFileName;

    char *DataBuffer;
    int DataBufferSize;
    int NumOfEvents;
    time_t StartTime=0;
    time_t EndTime=0;

    int ParseSetupFile(const std::string SetupFileName);
    int PMTNum = -999; // total number of PMTs
    int SigRes = 0;
    int PMTChMap[MAX_X742_CHANNEL_SIZE * MAX_X742_GROUP_SIZE]; // digitizer channel (0..31) for each PMT (0..PMTNum-1)
  std::vector<std::string> PMTNameMap;
  std::vector<std::string> PMTTitle;
    uint32_t TimeSamples = -999; // samples in one waveform
    float TimeRes = -999; // samples per second
  int CorrMask = -999;
//  std::string CorrFilesPath = "./";

    // int const PMTChannel[PMT_NUM] = PMT_CH_MAPPING;
 };
#endif



// EOF