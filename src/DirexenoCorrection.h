#include <fstream>
#include "TGraph.h"
#include "cnpy.h"
#ifndef DIREXENO_CORRECTION_H
#define DIREXENO_CORRECTION_H


class DirexenoCorrection
{
  //BLcap= (24, 1024)
  //BLtrig= (24, 900)

 public:
  DirexenoCorrection(const std::string setup_file_name, int TimeSamples_);
  //~DirexenoCorrection(){  std::cout<<"DirexenoCorrection class done."<<std::endl;}
  int* trig_init;
  int trig_init_value;
  const int nchannels=24; // not including the trigger channel
  const int length_BLcap = 1024;
  const int length_BLtrig = 900;
  TGraph *gcaps[24];
  TGraph *gcaps_shifted[24];

  TGraph *gtrig[24];
  double aa[24][1024];
  double *xxs;
  void draw_wf(float *wf, int n);
  void fix_SCA_baseline(float *wf, int start_index,int channel) ;

  double remove_baseline(float *wf, int N=10) ;
  void draw_BLcap();
  void draw_BLtrig();
 private:

  int TimeSamples;


};

#endif


