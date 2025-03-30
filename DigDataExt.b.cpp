// ==============================================================================
// Digitizer Data Extractor Class Implementation
// ==============================================================================
// Peter Szabo
// Created 02/09/2018
// Updated 31/12/2018
// Modified by Hagar on May 2023 - to include time stamps, and to generate monitoring info

// Add per channel : power by time



#include <cstdlib>

#include "DigDataExt.h"
#include <cmath>
#include <string>
#include "DirexenoCorrection.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TImage.h"
#include "TPaveStats.h"
#include "TGaxis.h"
#include "TROOT.h"
#include "TColor.h"
#include "TStyle.h"
#include "TGraph.h"
#include <cstring> // for strtok
#include <iomanip> // for std::quoted
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <limits> // for std::numeric_limits
#include <bitset>
#include "CAENDigitizerType.h" // #include "/home/home/access/Direxdaq/VME/CAENDigitizer-2.7.6/include/CAENDigitizerType.h"
#include "X742DecodeRoutines.DX.h"
#include "X742CorrectionRoutines.DX.h"
#include <sstream>
#include <vector>

using namespace std;


// Change back to have h2 with voltage and not power 
// subtract baseline from "Gera's plot"
// make a baseline subtracted version of other 2d plot
std::vector<std::string> splitString(const std::string& input, char delimiter) {
  std::vector<std::string> substrings;
  std::istringstream iss(input);
  std::string token;

  while (std::getline(iss, token, delimiter)) {
    substrings.push_back(token);
  }

  return substrings;
}

// Read and parse configuration file, set class parameters
int DigDataExt::ParseSetupFile(const std::string SetupFileName)
{
  // open setup file
    
  cout << endl <<  "Loading setup parameters from " << SetupFileName << ":" << endl;
  fstream SetupFile(SetupFileName, ios::in);
  if (!SetupFile) { cerr << "*** File opening error. ***" << endl << endl; exit(1); }

  // read text lines
  string line;
  bitset<10> param_set("0000000000");
  while(getline(SetupFile, line, '#'))
    {
      if (line.length() >= MAX_CFG_STR) { cerr << "*** Configuration file text line too long.  ***" << endl << endl; exit(1); }
      istringstream iss(line);
      string token;
      iss >> token;
      if (token == "PMT_NUM") 
	{ 
	  if (param_set[0]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[0] = 1;
	  iss >> PMTNum; 
	  cout << "PMT_NUM = \t" << PMTNum << endl; 
	}
      else if (token == "TIME_SAMPLES") 
	{ 
	  if (param_set[1]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[1] = 1;
	  iss >> TimeSamples; 
	  cout << "TIME_SAMPLES = \t" << TimeSamples << endl; 
	}
      else if (token == "TIME_RES") 
	{ 
	  if (param_set[2]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[2] = 1;
	  iss >> TimeRes; 
	  cout << "TIME_RES = \t" << TimeRes << " [Hz]" << endl; 
	}
      else if (token == "CORR_MASK")
	{
	  if (param_set[4]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[4] = 1;
	  string mask_str;
	  iss >> quoted(mask_str);
	  CorrMask = (int)bitset<3>(mask_str).to_ulong();
	  cout << "CORR_MASK = \t0d" << CorrMask << " = 0b" << bitset<3>(CorrMask) << "   (bits: |time|sample|cell|)" << endl; 
	}
      else if (token == "CORR_PATH") 
	{ 
	  if (param_set[5]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[5] = 1;
	  iss >> quoted(CorrFilesPath);
	  cout << "CORR_PATH = \t" << CorrFilesPath << endl; 
	}
      else if (token == "PMT_CH_MAP")
	{
	  if (param_set[6]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[6] = 1;
	  cout << "PMT_CH_MAP = \t" << left;
	  string list;
	  iss >> list;
	  istringstream iss_ch(list);
	  int channel;
	  char delim;
	  iss_ch >> delim;
	  if (delim != '[') { cerr << "*** PMT_CH_MAP channel array wrapped in square brackets. ***" << endl << endl; exit(1); }
	  int chn_cnt = 0;
	  while(iss_ch >> channel >> delim)
	    {
	      if (delim != ',' && delim != ']') { cerr << "*** PMT_CH_MAP channel array must be comma-delimited, no spaces or any other delimiters allowed. ***" << endl << endl; exit(1); }
	      PMTChMap[chn_cnt] = channel;
	      cout << setw(4) << PMTChMap[chn_cnt];
	      chn_cnt++;
	      if (delim == ']') { break; }
	    }
	  if (delim != ']') { cerr << "*** PMT_CH_MAP channel array must be comma-delimited and wrapped in square brackets. No spaces or any other delimiters allowed. ***" << endl << endl; exit(1); }
	  cout << " [channel no.]" << endl << "\t\t";
	  for (int i = 0; i < PMTNum; i++) { cout << setw(4) << i;}
	  cout << " [PMT no.]" << endl;
	  if (chn_cnt != PMTNum) { cerr << "*** Length of PMT_CH_MAP (" << chn_cnt << ") must be equal to PMT_NUM (" << PMTNum << "). ***" << endl << endl; exit(1); }
	}
      else if (token == "PMT_NAME_MAP")
	{
	  if (param_set[7]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[7] = 1;
	  cout << "PMT_NAME_MAP = \t" << left;
	  string list2;
	  iss >> list2; 
	  istringstream iss_ch(list2);
	  std::string name;
	  char delim2;
	  iss_ch >> delim2;
          cout<<delim2<<"=delim2,"<<endl;

	  if (delim2 != '[') { cerr << "***! PMT_NAME_MAP channel array wrapped in square brackets. ***" << endl << endl; exit(1); }
	  //	  int chn_cnt2 = 0;
	  cout<<delim2<<"=delim2,"<<endl;
	  iss_ch>>name;
	  char delimiter = ',';
	  //std::vector<std::string>
	  PMTNameMap = splitString(name, delimiter);
	  cout<<"sizesL"<<PMTNameMap.size()<<" "<<PMTTitle.size()<<endl;
	  for (int i=0; i<(int) PMTNameMap.size(); i++) {
	    cout<<i<<endl;
	    PMTTitle[i]+=" ";
	    PMTTitle[i]+=PMTNameMap[i];
	  }
	  cout<<"ok\n";
	  if ((int) PMTNameMap.size()!= (int)PMTNum)
	    { cerr << "*** Wrong number of names. PMT_NAME_MAP channel array must be comma-delimited and wrapped in square brackets. No spaces or any other delimiters allowed. ***" << endl << endl; exit(1); }
	  cout << " [channel name]" << endl << "\t\t";
	  cout << " [PMT no.]" << endl;
	  for (const auto& substring : PMTNameMap) {
	    std::cout << setw(4)<< substring;
	  }	 
	}

      else if (token == "") { /* skip, do nothing */ }
      else { /* skip, do nothing */ /*cerr << "*** Unknown configuration parameter " << token << " ***" << endl << endl; exit(1);*/ }
      SetupFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignore everything from "#" till end-of-line, start next getline in a new line
    }
  cout << endl;
  SetupFile.close();
  return 0;
}


// Load raw data, correct and serialize it, and write it to output file
DigDataExt::DigDataExt(const string setup_file_name, const string dig_file_name, const string out_file_name): SetupFileName(setup_file_name), DigFileName(dig_file_name), OutFileName(out_file_name)
{
  // load setup parameters
  for (int i=0; i<30; i++) PMTTitle.push_back(Form("Channel %d",i));
  cout<<PMTTitle.size()<<endl;
  cout << "Starting DigDataExt class." << endl;
  ParseSetupFile(SetupFileName);

  // load correction tables from files
  if (CorrFilesPath.length() > 0 && CorrFilesPath.back() != '/')
    {
      CorrFilesPath += '/';
    }
  cout << "Loading correction tables from:" << CorrFilesPath<< endl;
  CAEN_DGTZ_DRS4Correction_t* CorrTable[MAX_X742_GROUP_SIZE];
  for (int i = 0; i < MAX_X742_GROUP_SIZE; i++)
    {
      // CorrTable[i] = (CAEN_DGTZ_DRS4Correction_t*) malloc(sizeof(CAEN_DGTZ_DRS4Correction_t));
      CorrTable[i] = new CAEN_DGTZ_DRS4Correction_t;
      string s = CorrFilesPath + "X742Table_gr" + to_string(i);
      cout << s << "_*.txt   (* = cell, nsample, time)" << endl;
      if (0 > LoadCorrectionTable((char*)s.c_str(), CorrTable[i])) { cerr << "*** Correction table " << s << " read error. ***" << endl << endl; exit(1); }
    }
  float Tsamp;
  CAEN_DGTZ_DRS4Frequency_t SampFreq;

  if      (TimeRes == 5e9)   { SampFreq = CAEN_DGTZ_DRS4_5GHz;  Tsamp =(float)((1.0/5000.0)*1000.0); }
  else if (TimeRes == 2.5e9) { SampFreq = CAEN_DGTZ_DRS4_2_5GHz;  Tsamp =(float)((1.0/2500.0)*1000.0); }
  else if (TimeRes == 1e9)   { SampFreq = CAEN_DGTZ_DRS4_1GHz;  Tsamp =(float)((1.0/1000.0)*1000.0); }
  else if (TimeRes == 7.5e8) { SampFreq = CAEN_DGTZ_DRS4_750MHz;  Tsamp =(float)((1.0/750.0)*1000.0);  }
  else { cerr << "*** Unknown sampling frequency. ***" << endl << endl; exit(1); }

  // open digitizer data file
  if (4 != sizeof(float)) { cerr << "*** sizeof(float) is not 4 bytes, but " << sizeof(float) << " bytes. ***" << endl << endl; exit(1); }
  cout << "Opening digitizer raw data file " << DigFileName << " ." << endl;
  fstream DigDataFile(DigFileName, ios::in|ios::binary);
  if (!DigDataFile) { cerr << "*** File opening error. ***" << endl << endl; exit(1); }
  DigDataFile.seekg(0, DigDataFile.end);
  DataBufferSize = DigDataFile.tellg();
  DigDataFile.seekg(0);
 
  // read raw data to memory
  DataBuffer = new char[DataBufferSize];
  if (nullptr == DataBuffer) { cerr << "*** Memory allocation error (DataBuffer). ***" << endl << endl; exit(1); }
  DigDataFile.read(DataBuffer, DataBufferSize);
  if (!DigDataFile.good()) { cerr << "*** Data read error (DataBuffer). ***" << endl << endl; exit(1); }
  GetNumEvents(DataBuffer, DataBufferSize, (unsigned int*)&NumOfEvents);
  NumOfEvents = NumOfEvents - 1; // dump last event (often corrupt)
  cout << "File has " << int(NumOfEvents) << " events (last event was dumped)." << endl;

  // open output data file
  cout << "Opening output data file " << OutFileName << " ." << endl;
  fstream OutDataFile(OutFileName, ios::out|ios::app|ios::binary);
  if (!OutDataFile) { cerr << "*** File opening error. ***" << endl << endl; exit(1); }

  // extract, correct and write data to output file
  cout << "Correcting and serializing events to file:" << endl;
  //  long int previous_time_tag = 0;

  TH1F *h_dt = new TH1F("h_dt","Time differences ",1000,0,1e4);
  TH1F *h_ch_v[PMTNum+ MAX_X742_GROUP_SIZE];
  TH1F *h_ch_sigma[PMTNum+ MAX_X742_GROUP_SIZE];
  TH1F *h_ch_wf_bins[PMTNum+ MAX_X742_GROUP_SIZE];
  TH2F *h2_ch_v[PMTNum+ MAX_X742_GROUP_SIZE];
  TH2F *h2_ch_wf_bins[PMTNum+ MAX_X742_GROUP_SIZE];

  // Holds snapshots of waveforms:
  int Nsamples = 5;
  TGraph *gwf_sample[Nsamples][PMTNum+ MAX_X742_GROUP_SIZE];
  TGraph *gtimes=new TGraph();
  gtimes->GetXaxis()->SetTitle("Time since event 0 [usec]");
  gtimes->GetYaxis()->SetTitle("Event number");
  TGraph *gtimes_dt=new TGraph();
  gtimes_dt->GetXaxis()->SetTitle("Time since event 0 [usec]");
  gtimes_dt->GetYaxis()->SetTitle("dt from previous  event [usec]");
  TGraph *gpower_dt=new TGraph();
  gpower_dt->GetXaxis()->SetTitle("Time since event 0 [usec]");
  gpower_dt->GetYaxis()->SetTitle("Total voltage [mv]");
  TGraph *gpower2_dt=new TGraph();
  gpower2_dt->GetXaxis()->SetTitle("Time since event 0 [usec]");
  gpower2_dt->GetYaxis()->SetTitle("Total power [~mv^2]");
  TGraph *gpower2_dt_ch[PMTNum+ MAX_X742_GROUP_SIZE];
  /*
  for (int ch=0; ch<PMTNum+ MAX_X742_GROUP_SIZE;ch++)
    {
      //      h_ch_v[ch]=new TH1F(Form("ch%d",ch),Form("ADC distribution ch%d; Count [mVolts]; Entries",ch),4096,-2050,2050);
      h_ch_v[ch]=new TH1F(Form("ch%d",ch),Form("ADC distribution ch%d; Count [mVolts]; Entries",ch),4096,0,4096);
      h_ch_v[ch]->SetTitle(Form("ADC distribution ch%s",PMTTitle[ch].c_str()));
      h_ch_v[ch]->GetXaxis()->SetTitle("Counts ADC");
      h_ch_v[ch]->GetYaxis()->SetTitle("Entries");
      h_ch_sigma[ch]=new TH1F(Form("h_sigma_%s",PMTTitle[ch].c_str()),Form("Consecutive over sigma %d",ch),10,0,11);
      h_ch_sigma[ch]->SetTitle(Form("Consecutive over sigma %d",ch));
      h_ch_sigma[ch]->GetXaxis()->SetTitle("consecutive Sigmas ");
      h_ch_sigma[ch]->GetYaxis()->SetTitle("Entries");
      gpower2_dt_ch[ch]=new TGraph();
      gpower2_dt_ch[ch]->SetTitle(Form(" - %s",PMTTitle[ch].c_str()));
      gpower2_dt_ch[ch]->GetXaxis()->SetTitle("Time since event 0 [usec]");
      gpower2_dt_ch[ch]->GetYaxis()->SetTitle("Total power [~mv^2]");
      for (int cn=0; cn<Nsamples; cn++) {
	gwf_sample[cn][ch]=new TGraph(); gwf_sample[cn][ch]->SetTitle(Form("WF sample %d - %s",cn,PMTTitle[ch].c_str()));      
      }
    }
  */
  cout<<"created histograms "<<PMTNum<<endl;
  
  //  NumOfEvents=10000;
  int Nmax = NumOfEvents; //NumOfEvents/3;// /30;
  //    Nmax = 1000;
  //  long int first_time_tag = -999;
  long int roundTime=pow(2,30);
  //long int roundTime_usec=roundTime*8.5/1000;
  CAEN_DGTZ_X742_EVENT_t *Event;
  char *EventPtr;
  DirexenoCorrection *dxc = new DirexenoCorrection("./210223_BL.npz", TimeSamples);
  long int preTime = -999;
  long int preTime_usec = -999;
  long int T_first = -999;
  int tloop = 0;
  // Get the duration of the run....this might take a while:
  printf ("looping on all events to get run duration....this might take a few secondsa....\n");

  if (1==1) {
    for (int e = 0; e < Nmax; e++)
      {
	EventPtr = NULL;
	GetEventPtr(DataBuffer, DataBufferSize, e, &EventPtr);
	Event = NULL;
      
	X742_DecodeEvent(EventPtr, (void **)&Event);  // <===== memory leak is here
	long int TimeTag = (long int)Event->DataGroup[0].TriggerTimeTag;
	if (preTime!=-999) {
	  if (TimeTag+tloop*roundTime < preTime) {
	    tloop++;
	    cout<<"tloop="<<tloop<<endl;
	  }
	}
	if (T_first == -999) T_first = TimeTag;      
	TimeTag = TimeTag + tloop*roundTime;
	preTime = TimeTag;
      }
  }
  double_t T_last_usec = preTime*8.5/1000;
  double_t T_first_usec = T_first*8.5/1000;
  
  //double_t T_last_usec=3.32157e7;
  //double_t T_first_usec=0;
  // cout<<"using fixed valuees for t_first and t_last \n";
  cout<<"Tfirst = "<<(T_first_usec)<<"Tlast = "<<(T_last_usec)<<"  dt="<<(T_last_usec - T_first_usec)<<" n  = "<<tloop<<endl;;
  int nbins = 1e-6*(T_last_usec - T_first_usec) ; // 2d histogram bin size is 0.1 sec
  if (nbins<5) nbins=10;
  if (nbins>1000) nbins=1000;
  cout<<T_last_usec-T_first_usec<<endl;
  cout<<1e-6*(T_last_usec-T_first_usec)<<endl;
  
    
  cout<<" nbins= "<<nbins<<endl;
  for (int ch=0; ch<PMTNum+ MAX_X742_GROUP_SIZE;ch++)
    {
      h_ch_v[ch]=new TH1F(Form("ch%d",ch),Form("ADC distribution ch%d; Counts ADC [mVolts]; Entries",ch),4096,0,4096);
      h_ch_sigma[ch]=new TH1F(Form("h_sigma_%s",PMTTitle[ch].c_str()),Form("Consecutive over sigma %d; consecutive sigmas, Entries",ch),10,0,11);
      h_ch_wf_bins[ch]= new TH1F(Form("ch%d_wf",ch),Form("overall bin  distribution ch%d; [ns]; Entries",ch),(TimeSamples -20)/4, 0, (TimeSamples-20)*Tsamp);
   
      gpower2_dt_ch[ch]=new TGraph();
      gpower2_dt_ch[ch]->SetTitle(Form(" - %s",PMTTitle[ch].c_str()));
      gpower2_dt_ch[ch]->GetXaxis()->SetTitle("Time since event 0 [usec]");
      gpower2_dt_ch[ch]->GetYaxis()->SetTitle("Total power [~mv^2]");
      for (int cn=0; cn<Nsamples; cn++) {
	gwf_sample[cn][ch]=new TGraph(); gwf_sample[cn][ch]->SetTitle(Form("WF sample %d - %s",cn,PMTTitle[ch].c_str()));      
      }	
      h2_ch_v[ch]=new TH2F(Form("ch%d_2d",ch),Form("ADC distribution ch%d by time; TimeSince event 0 [usec]; Count [mVolts]",ch),nbins,T_first_usec,T_last_usec,4096,0,4096);
      h2_ch_wf_bins[ch] = new TH2F(Form("ch%d_2d_wf",ch),Form("WF distribution ch%d by time; TimeSince event 0 [usec];  [ns]",ch),nbins,T_first_usec,T_last_usec,(TimeSamples-20)/4,0,(TimeSamples-20)*Tsamp);

      //	h2_ch_v[ch]=new TH2F(Form("ch%d_2d",ch),Form("ADC distribution ch%d by time; TimeSince event 0 [usec]; Count [mVolts]",ch),nbins,T_first_usec,T_last_usec,2100,-100,2000);
      //	h2_ch_v[ch]=new TH2F(Form("ch%d_2d",ch),Form("ADC distribution ch%d by time; TimeSince event 0 [usec]; Count [mVolts]",ch),nbins,T_first_usec,T_last_usec,2100,0,2000*2000);
      h2_ch_v[ch]->SetTitle(Form("Power distribution ch%s by time",PMTTitle[ch].c_str()));
      h2_ch_v[ch]->GetXaxis()->SetTitle("Time since event 0 [usec]");
      h2_ch_v[ch]->GetYaxis()->SetTitle("sum of v^2");
    }
  tloop=-1;
  for (int e = 0; e < Nmax; e++)
    {
      Float_t tot_power = 0;
      Float_t tot_power2 = 0;
      EventPtr = NULL;
      double baselines[PMTNum+ MAX_X742_GROUP_SIZE];
      GetEventPtr(DataBuffer, DataBufferSize, e, &EventPtr);
      Event = NULL;
      X742_DecodeEvent(EventPtr, (void **)&Event);  // <===== memory leak is here
      // correct data
      for (int j = 0; j < MAX_X742_GROUP_SIZE; j++)
	{
	  ApplyDataCorrection(CorrTable[j], SampFreq, CorrMask, &Event->DataGroup[j]);
	}
     
      
      // ch is the number of pmt.   c,g is the number of the channel on digitizer
      for (int ch = 0; ch < (PMTNum + MAX_X742_GROUP_SIZE); ch++)	{
	int g, c;
	if (ch < PMTNum) { // PMT channel 
	  g = PMTChMap[ch] / (int)(MAX_X742_CHANNEL_SIZE - 1); // MAX_X742_CHANNEL_SIZE = 9 due to the fast trigger channels (see CAENDigitizerType.h)
	  c = PMTChMap[ch] % (int)(MAX_X742_CHANNEL_SIZE - 1);
	}
	else { // fast trigger ch annel
	  g = ch - PMTNum;
	  c = MAX_X742_CHANNEL_SIZE - 1;
	}
	baselines[ch] = dxc->remove_baseline(Event->DataGroup[g].DataChannel[c], 50);
	baselines[ch] = 0;
      } // Finished looping on all PMTs to apply corrections

      // first write PMT channel data to file by PMT order, and then write the four fast-trigger channels
      long int TimeTag = (long int)Event->DataGroup[0].TriggerTimeTag;
      long int TimeTag_usec = (long int)Event->DataGroup[0].TriggerTimeTag*8.5/1000;

      for (int g = 1; g<4; g++) 
	if (TimeTag !=  (long int)Event->DataGroup[g].TriggerTimeTag)
	  cout<<"*************     Time tags of groups are different   ******"<<endl;

      if (preTime!=-999) {
	if (TimeTag+tloop*roundTime < preTime) tloop++;
      }
      if (T_first == -999) T_first = TimeTag;      
      TimeTag = TimeTag + tloop*roundTime;
      TimeTag_usec = TimeTag * 8.5/1000;
      if (e>0)
	{
	  h_dt->Fill(TimeTag_usec - preTime_usec);
	  //	      h_dt->Fill(preTime_usec - TimeTag_usec);
	  gtimes_dt->SetPoint(gtimes_dt->GetN(),TimeTag_usec - T_first_usec, TimeTag_usec - preTime_usec);
	  // gtimes_dt->SetPoint(gtimes_dt->GetN(),event_dt0, event_dt);
	  gtimes->SetPoint(gtimes->GetN(),TimeTag_usec - T_first_usec, e);
	  //	      gtimes->SetPoint(gtimes->GetN(),event_dt0, e);
	      
	}
      //}
      //previous_time_tag = TimeTag;
      preTime = TimeTag;
      preTime_usec = preTime*8.5/1000;
    
    
      for (int ch = 0; ch < (PMTNum + MAX_X742_GROUP_SIZE); ch++)
	{
	  int g,c;
	  if (ch < PMTNum) // PMT channel
	    { 
	      g = PMTChMap[ch] / (int)(MAX_X742_CHANNEL_SIZE - 1); // MAX_X742_CHANNEL_SIZE = 9 due to the fast trigger channels (see CAENDigitizerType.h)
	      c = PMTChMap[ch] % (int)(MAX_X742_CHANNEL_SIZE - 1);
	    }
	  else // fast trigger channel
	    {
	      g = ch - PMTNum;
	      c = MAX_X742_CHANNEL_SIZE - 1;
	    }
	  // check data length integrity (all data vectors should be TimeSamples in length)
	  if (TimeSamples != Event->DataGroup[g].ChSize[c]) { cerr << "*** Event " << e << " PMT/TR " << ch << " data length is not " << TimeSamples << " ***" << endl << endl; exit(1); }

	  // save data to file
	  float StartIndex = (float)Event->DataGroup[g].StartIndexCell;
	  //	  cout<<" channel="<<ch<<"  group="<<g<<"   i="<<StartIndex<<endl;
      
	  OutDataFile.write((char*)&TimeTag, sizeof(float));
	  OutDataFile.write((char*)&StartIndex, sizeof(float));
	  OutDataFile.write((char*)Event->DataGroup[g].DataChannel[c], TimeSamples * sizeof(float));

	  float tot_power2_ch=0;
	  TGraph *gtemp = new TGraph();
	  //  cout<<"=====> "<<Event->DataGroup[g].DataChannel[c][30];
	  
	  //  cout<<"=====> "<<Event->DataGroup[g].DataChannel[c][30]<<endl;
	  
	  for (int b=0; b<(int) TimeSamples-20; b++){
	    gtemp->SetPoint(gtemp->GetN(),b,Event->DataGroup[g].DataChannel[c][b]);
	    h_ch_v[ch]->Fill(Event->DataGroup[g].DataChannel[c][b]);
	    //cout<<ch<<"\t"<<TimeTag_usec + tloop*roundTime_usec<<"\t"<< Event->DataGroup[g].DataChannel[c][b]<<endl;
	    h2_ch_v[ch]->Fill(TimeTag_usec, Event->DataGroup[g].DataChannel[c][b] );
	    if (b>2 && b<TimeSamples-40) h2_ch_wf_bins[ch]->Fill(TimeTag_usec, b*Tsamp, Event->DataGroup[g].DataChannel[c][b] );
	    if (b>2 && b<TimeSamples-40) h_ch_wf_bins[ch]->Fill(b*Tsamp,Event->DataGroup[g].DataChannel[c][b] );    
	    tot_power = tot_power+Event->DataGroup[g].DataChannel[c][b];
	    tot_power2 = tot_power2+Event->DataGroup[g].DataChannel[c][b]*Event->DataGroup[g].DataChannel[c][b];
	    tot_power2_ch = tot_power2_ch+Event->DataGroup[g].DataChannel[c][b]*Event->DataGroup[g].DataChannel[c][b];
	  }
	  double* y = gtemp->GetY();
	  //for positive waveform
	  //int locmax = TMath::LocMax(gtemp->GetN(),y);
	  // int sign = +1;
	  
	  // for negative wafform
	  int locmax = TMath::LocMin(gtemp->GetN(),y);
	  int sign =-1;
	    
	  double h_mean = gtemp->GetMean(2);
	  double h_stdev = gtemp->GetRMS(2);
	  int oks[10];
	  for (int iok=0; iok<10; iok++) oks[iok]=0;
	  //int ok7=0;
	  // int ok4=0;
	  // Add counting of number of events with sigma over entire run per channel
	  for (int db = locmax-5; db<locmax+5; db++) {
	    if (db>=0 && db<gtemp->GetN()) {
	      float v = gtemp->GetY()[db];
	      for (int iok=9; iok>0;iok--) {
		if (v < h_mean + sign* iok * h_stdev) {
		  oks[iok]++;
		  break;
		}
	      }
	    }
	  }
      
	  delete gtemp;
	  int index = -999;
	  int sig1=6;
	  int sig2=3;
	  if (oks[sig1]>6) {
	    index = 4;
	    gwf_sample[index][ch]->SetTitle(Form("More than %d sigmas, WF %d - %s  (mn=%.2f,stv=%.2f)",sig1,e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	  }
	  if (oks[sig2]>6  && index==-999) {
	    index = 3;
	    gwf_sample[index][ch]->SetTitle(Form("More than %d sigmas, WF %d - %s  (mn=%.2f,stv=%.2f)",sig2,e,PMTTitle[ch].c_str(),h_mean,h_stdev));

	  }
	  int n_consecutive=6;
	  for (int iok=0; iok<10; iok++) if (oks[iok]>n_consecutive) h_ch_sigma[ch]->Fill(iok);
	  h_ch_sigma[ch]->SetTitle(Form("at least %d consecutive bins are x sig above baseline, ch%d",n_consecutive,ch));
	
	  if (e==0) {
	    index=0;
	    gwf_sample[index][ch]->SetTitle(Form("WF %d - %s  (mn=%.2f,stv=%.2f)",e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	  }
	  if (e==1 ) {
	    index=1;
	    gwf_sample[index][ch]->SetTitle(Form("WF %d - %s  (mn=%.2f,stv=%.2f)",e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	  }
	  if (e==12 || e==Nmax-1) {
	    index=2;
	    gwf_sample[index][ch]->SetTitle(Form("WF %d - %s  (mn=%.2f,stv=%.2f)",e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	  }
	  if (index!=-999)
	    {
	      for (uint32_t b=0; b<TimeSamples-20; b++)
		{
		  gwf_sample[index][ch]->SetPoint(b, Tsamp * b,Event->DataGroup[g].DataChannel[c][b]);
		
		}
	    }
	  
	  if(!OutDataFile.good()) { cerr << "*** File writing error. ***" << endl << endl; exit(1); }
	  gpower2_dt_ch[ch]->SetPoint(gpower2_dt_ch[ch]->GetN(),  TimeTag_usec - T_first_usec, tot_power2_ch);
	} // End of Loop on PMTs channels
      gpower_dt->SetPoint(gpower_dt->GetN(), TimeTag_usec - T_first_usec , tot_power);
      gpower2_dt->SetPoint(gpower2_dt->GetN(), TimeTag_usec - T_first_usec, tot_power2);    
      // free memory allocated by X742_DecodeEvent
      char chanMask=15;

      // This is a fix to the memory leak. HYL 20/5/2023
      for (int g=0; g<X742_MAX_GROUPS; g++) {
	if ((chanMask >> g) & 0x1) {
	  for (int j=0; j<MAX_X742_CHANNEL_SIZE; j++) {
	    // Event->DataGroup[g].DataChannel[j]= malloc(X742_FIXED_SIZE * sizeof (float));
	    free(Event->DataGroup[g].DataChannel[j]);
	  }
	}
      }
      // Fix ended
      
      delete Event;
      cout << "\r";
      int PrgBarWidth = 50;
      float Prg = (e+1) / (float)NumOfEvents;
      cout << "[";
      int PrgPos = PrgBarWidth * Prg;
      for (int i = 0; i < PrgBarWidth; ++i)
	{
	  if (i < PrgPos) cout << "=";
	  else if (i == PrgPos) cout << ">";
	  else cout << " ";
	}
      cout << "] " << int(Prg * 100.0) << " % (" << (e+1) << " events)";
      cout.flush();
    
    } // End of Loop on e Nevents 
  cout << endl; // end the progress bar's last line update

  // free correction tables memory
  for (int i=0; i<MAX_X742_GROUP_SIZE; i++) 
    {
      // free(CorrTable[i]);
      delete CorrTable[i];
    }
  
  // close and finish
  DigDataFile.close();
  OutDataFile.close();
  TFile* myfile = TFile::Open("file.root","recreate");
  //  ( TFile::Open("file.root", "RECREATE") );
  myfile->WriteObject(h_dt, "h_dt");
  gtimes->Write("gtimes");
  gtimes_dt->Write("gtimes_dt");
  gpower_dt->Write("gpower_dt");
  gpower2_dt->Write("gpower2_dt");
  

  float x_range_width = 300; //for graphics. Will try to keep xaxis to this range

  // Summarize individual channels monitoring information: Fits, write
  double ch_mean[PMTNum + MAX_X742_GROUP_SIZE];
  double ch_stdev[PMTNum + MAX_X742_GROUP_SIZE];
  int voltage_lastbin[25];
  int voltage_firstbin[25];

  for (int ch=0; ch<PMTNum + MAX_X742_GROUP_SIZE; ch++){ // skip the first channel (trigger)
    float b_first=0;
    float b_last=0;
    int binmax = h_ch_v[ch]->GetMaximumBin();
    for (int b=0; b<h_ch_v[ch]->GetNbinsX(); b++) {
      float v = h_ch_v[ch]->GetBinContent(b);
      if (b_first==0 && v>0) b_first=h_ch_v[ch]->GetBinCenter(b-1);
      if (v>0) b_last=h_ch_v[ch]->GetBinCenter(b+1);;
    }
    voltage_lastbin[ch] = b_last;
    voltage_firstbin[ch] = b_first;
    
    if (b_last - b_first < x_range_width) {
      float delta =  x_range_width - (b_last - b_first);
      h_ch_v[ch]->GetXaxis()->SetRangeUser(b_first-delta/2, b_last+delta/2);
    }
    else
      {
	h_ch_v[ch]->GetXaxis()->SetRangeUser(b_first-30, b_last+30);
      }
    h_ch_v[ch]->Fit("gaus", "Q","",  h_ch_v[ch]->GetXaxis()->GetBinCenter(binmax-50),  h_ch_v[ch]->GetXaxis()->GetBinCenter(binmax+50));
    // cout<<binmax<<"==binmax"<<endl;
    TF1 *fit = h_ch_v[ch]->GetFunction("gaus");
    //int fitok=0;
    ch_mean[ch] = h_ch_v[ch]->GetMean();
    ch_stdev[ch] = 20;
    if (fit) {
      //fitok=1;
      fit->SetLineStyle(2);
      ch_mean[ch] = fit->GetParameter(1);
      ch_stdev[ch] = fit->GetParameter(2);
      h_ch_v[ch]->Write();
      gpower2_dt_ch[ch]->Write(Form("gpower2_dt_%d",ch));
      //gwf_sample1[ch]->Write(Form("gwf_sample1_%d",ch));
      //gwf_sample2[ch]->Write(Form("gwf_sample2_%d",ch));
      //gwf_sample3[ch]->Write(Form("gwf_sample3_%d",ch));
      // gwf_sample4[ch]->Write(Form("gwf_sample4_%d",ch));
    }
  }


  //  gwf_sample1[ch];
  int numColors = 25;
  //int colors[numColors] = {TColor::GetColor(0, 0, 139)  ,TColor::GetColor(220, 20, 60)  ,TColor::GetColor(0, 100, 0)  ,TColor::GetColor(255, 165, 0)  ,TColor::GetColor(75, 0, 130)  ,TColor::GetColor(178, 34, 34)  ,TColor::GetColor(0, 139, 139)  ,TColor::GetColor(199, 21, 133)  ,TColor::GetColor(128, 128, 0)  ,TColor::GetColor(0, 128, 128)  ,TColor::GetColor(210, 105, 30)  ,TColor::GetColor(153, 50, 204)  ,TColor::GetColor(255, 127, 80)  ,TColor::GetColor(47, 79, 79)  ,TColor::GetColor(255, 99, 71)  ,TColor::GetColor(65, 105, 225)  ,TColor::GetColor(139, 69, 19)  ,TColor::GetColor(60, 179, 113)  ,TColor::GetColor(70, 130, 180)  ,TColor::GetColor(160, 82, 45)  ,TColor::GetColor(30, 144, 255)  ,TColor::GetColor(205, 133, 63)  ,TColor::GetColor(147, 112, 219)  ,TColor::GetColor(102, 205, 170)  ,TColor::GetColor(205, 92, 92)};
  int colors[numColors] = {
    TColor::GetColor(255, 0, 0),        // Red
    TColor::GetColor(255, 165, 0),      // Orange
    TColor::GetColor(255, 255, 0),      // Yellow
    TColor::GetColor(0, 255, 0),        // Green
    TColor::GetColor(0, 255, 255),      // Cyan
    TColor::GetColor(0, 0, 255),        // Blue
    TColor::GetColor(75, 0, 130),       // Indigo
    TColor::GetColor(148, 0, 211),      // Violet
    TColor::GetColor(139, 0, 0),        // Dark Red
    TColor::GetColor(255, 140, 0),      // Dark Orange
    TColor::GetColor(204, 204, 0),      // Dark Yellow
    TColor::GetColor(0, 100, 0),        // Dark Green
    TColor::GetColor(0, 139, 139),      // Dark Cyan
    TColor::GetColor(0, 0, 139),        // Dark Blue
    TColor::GetColor(83, 0, 104),       // Dark Indigo
    TColor::GetColor(139, 0, 139),      // Dark Violet
    TColor::GetColor(255, 102, 102),    // Light Red
    TColor::GetColor(255, 204, 153),    // Light Orange
    28,
    //TColor::GetColor(255, 255, 153),    // Light Yellow
    TColor::GetColor(204, 255, 153),    // Light Green
    // TColor::GetColor(153, 255, 255),    // Light Cyan
    13,
    TColor::GetColor(153, 204, 255),    // Light Blue
    TColor::GetColor(173, 216, 230),    // Light Indigo
    TColor::GetColor(238, 130, 238),    // Light Violet
    TColor::GetColor(255, 192, 203)     // Pink
  };

  //moshe

  // Fill 2d histogram

	 
  TCanvas * c_power_2d_a= new TCanvas(Form("c_power_2d_a"),Form("c_power_2d_a"),3600,3600);
  c_power_2d_a->Divide(5,5);
  for (int i=0; i<25; i++) {
    c_power_2d_a->cd(i+1);
    //h_ch_sigma[i]->SetLineWidth(2);
    gPad->SetLogy(0);
    gPad->SetLogz(1);
    gPad->SetGrid();
    Int_t lastBinY = h2_ch_v[i]->GetYaxis()->GetLast();
    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->GetFirst();
    //firstBinY=1;
    //    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->GetFirst();
    //cout<<" Bins  are = "<<voltage_firstbin[i]<<"\t"<<voltage_lastbin[i]<<endl;
    h2_ch_v[i]->GetYaxis()->SetRange(voltage_firstbin[i]-20,voltage_lastbin[i]+20);
    h2_ch_v[i]->Draw("colz");
    
    //    h2_ch_v[i]->GetYaxis()->SetRange(firstBinY-5, lastBinY+5);
    gPad->Update();
    h2_ch_v[i]->Write(Form("h2_ch_sigma%d",i));
  }
  c_power_2d_a->SaveAs(Form("./c_power_2d_a.png"));
  c_power_2d_a->Write(Form("c_power_2d_a"));

  TCanvas * c_power_2d_b= new TCanvas(Form("c_power_2d_b"),Form("c_power_2d_b"),3600,3600);
  c_power_2d_b->Divide(5,5);
  for (int i=0; i<25; i++) {
    c_power_2d_b->cd(i+1);
    
    //h_ch_sigma[i]->SetLineWidth(2);
    gPad->SetLogy(0);
    gPad->SetLogz(1);
    gPad->SetGrid();
    Int_t lastBinY = h2_ch_v[i]->GetYaxis()->FindBin( ch_mean[i] + 10*ch_stdev[i]);
    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->FindBin( ch_mean[i] - 10*ch_stdev[i]);
    //Int_t firstBinY = h2_ch_v[i]->GetYaxis()->FindBin(30);
    //    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->FindBin(1);
    //    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->GetFirst();
    //h2_ch_v[i]->GetYaxis()->SetRangeUser(-50,50);
    h2_ch_v[i]->GetYaxis()->SetRange(firstBinY,lastBinY);

    h2_ch_v[i]->Draw("colz");
    gPad->Update();
    gPad->Update();
    //    h2_ch_v[i]->Write(Form("h2_ch_sigma%d",i));
  }
  c_power_2d_b->SaveAs(Form("./c_power_2d_b.png"));
  c_power_2d_b->Write(Form("c_power_2d_b"));

  
  TCanvas * c_ch_wf_bin_2d_b= new TCanvas(Form("c_ch_wf_bin_2d_b"),Form("c_ch_wf_bin_2d_bb"),3600,3600);
  c_ch_wf_bin_2d_b->Divide(5,5);
  for (int i=0; i<25; i++) {
    c_ch_wf_bin_2d_b->cd(i+1);    
    gPad->SetGrid();
    h2_ch_wf_bins[i]->Draw("colz");
    gPad->Update();
    gPad->Update();
    h2_ch_wf_bins[i]->Write(Form("h2_ch_wf_bins%d",i));
  }
 c_ch_wf_bin_2d_b->SaveAs(Form("./c_ch_wf_bin_2d_b.png"));
  c_ch_wf_bin_2d_b->Write(Form("c_ch_wf_bin_2d_b"));

  TCanvas * c_ch_wf_bin_1d= new TCanvas(Form("c_ch_wf_bin_1d"),Form("c_ch_wf_bin_1d"),3600,3600);
  c_ch_wf_bin_1d->Divide(5,5);
  for (int i=0; i<25; i++) {
    c_ch_wf_bin_1d->cd(i+1);    
    gPad->SetGrid();
    h_ch_wf_bins[i]->Draw();
    gPad->Update();
    gPad->Update();
    h_ch_wf_bins[i]->Write(Form("h_ch_wf_bins%d",i));
  }
  c_ch_wf_bin_1d->SaveAs(Form("./c_ch_wf_bin_1d.png"));
  c_ch_wf_bin_1d->Write(Form("c_ch_wf_bin_1d"));
		
  /*
    TCanvas * c_power_2d_c= new TCanvas(Form("c_power_2d_c"),Form("c_power_2d_c"),3600,3600);
    c_power_2d_c->Divide(5,5);
    for (int i=0; i<25; i++) {
    c_power_2d_c->cd(i+1);
    //h_ch_sigma[i]->SetLineWidth(2);
    gPad->SetLogy(0);
    gPad->SetGrid();
    Int_t lastBinY = h2_ch_v[i]->GetYaxis()->FindBin(1000);
    //    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->FindBin(1);
    //    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->GetFirst();
    h2_ch_v[i]->GetYaxis()->SetRange(1, lastBinY+10);

    h2_ch_v[i]->Draw("colz");
    gPad->Update();
    gPad->Update();
    h2_ch_v[i]->Write(Form("h2_ch_sigma%d",i));
    }
    c_power_2d_c->SaveAs(Form("./c_power_2d_c.png"));
    c_power_2d_c->Write(Form("c_power_2d_c"));
  */
  TCanvas *c1 = new TCanvas("c1","c1",2400,1600);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(111);
  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.01);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetTitleOffset(0.01,"xyz");
  c1->Divide(5,5);
  for (int i=0; i<25; i++) {
    c1->cd(i+1);
    gPad->SetLogy();
    gPad->SetGrid();
    h_ch_v[i]->SetTitleSize(0.08);
    h_ch_v[i]->GetYaxis()->SetLabelSize(0.06);
    h_ch_v[i]->GetXaxis()->SetLabelSize(0.06);
    h_ch_v[i]->GetYaxis()->SetTitleSize(0.04);
    h_ch_v[i]->GetXaxis()->SetTitleSize(0.04);
    h_ch_v[i]->SetLineColor(colors[i]);
    h_ch_v[i]->SetMarkerColor(colors[i]);
   
    h_ch_v[i]->Draw();
    gPad->Update();
    TPaveStats *stats = (TPaveStats*) gPad->GetPrimitive("stats");
    if (stats) {
      stats->SetFillStyle(0);
      stats->SetX1NDC(0.1); //new x start position
      stats->SetX2NDC(0.5); //new x end position
      stats->SetY1NDC(0.6); //new x start position
      stats->SetY2NDC(0.9); //new x end position
      //    stats->SetY1NDC(0.5); //new x start position
      //stats->SetY2NDC(1); //new x end position
      gPad->Update();
    }
  
  }
  c1->SaveAs("./c1.png");
  c1->Write("c1");
  TCanvas *c_wf[4];
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(111);
  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.01);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetTitleOffset(0.01,"xyz");

  for (int cn=0; cn<Nsamples; cn++){
    c_wf[cn]= new TCanvas(Form("c_wf%d",cn),Form("c_wf%d",cn),2400,1600);
    c_wf[cn]->Divide(5,5);
    for (int i=0; i<25; i++) {
      c_wf[cn]->cd(i+1);
      gPad->SetLogy(0);
      gPad->SetGrid();
      gwf_sample[cn][i]->SetLineColor(colors[i]);
      gwf_sample[cn][i]->SetMarkerColor(colors[i]);
      if (gwf_sample[cn][i]->GetN()>0)   gwf_sample[cn][i]->Draw("Ac");
      dxc->gcaps_shifted[i]->Draw("c");
      gPad->Update();
      gwf_sample[cn][i]->Write(Form("wf_Sample%d_ch%d",cn,i));
    }
    c_wf[cn]->Write(Form("c_wf%d",cn));
  }

  c_wf[0]->SaveAs(Form("./c_wf_first.png"));
  c_wf[1]->SaveAs(Form("./c_wf_mid.png"));
  c_wf[2]->SaveAs(Form("./c_wf_last.png"));
  c_wf[3]->SaveAs(Form("./c_wf_sigmaL.png"));
  c_wf[4]->SaveAs(Form("./c_wf_sigmaH.png"));

  TCanvas * c_sigma= new TCanvas(Form("c_sigma"),Form("c_sigma"),2400,1600);
  c_sigma->Divide(5,5);
  for (int i=0; i<25; i++) {
    c_sigma->cd(i+1);
    h_ch_sigma[i]->SetLineWidth(2);
    gPad->SetLogy(1);
    gPad->SetGrid();
    h_ch_sigma[i]->SetLineColor(colors[i]);
    h_ch_sigma[i]->SetMarkerColor(colors[i]);
    
    h_ch_sigma[i]->Draw();
    gPad->Update();
    gPad->Update();
    h_ch_sigma[i]->Write(Form("h_ch_sigma%d",i));
  }
  c_sigma->SaveAs(Form("./c_sigma.png"));
  c_sigma->Write(Form("c_sigma"));
  

 

  TCanvas *c3 = new TCanvas("c3","c3",2400,1600);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(111);
  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.01);
  gStyle->SetPadBottomMargin(0.1);
  //gStyle->SetTitleSize(0.02,"xyz");
  gStyle->SetTitleOffset(0.01,"xyz");
  /*gStyle->SetPadRightMargin(0.01);
    gStyle->SetPadLeftMargin(0.05);
    gStyle->SetPadRightMargin(0.05);
    gStyle->SetPadBottomMargin(0.05);
    gStyle->SetPadTopMargin(0.05);
    gStyle->SetLabelFont(132,"xyz");
    gStyle->SetLabelSize(0.08,"xyz");
    gStyle->SetTitleFont(132,"xyz");
    gStyle->SetNdivisions(505,"xyz");    // less ticks and numbers on axis (this is VERY handy!)
  */
  c3->Divide(5,5);
  for (int i=0; i<25; i++) {
    c3->cd(i+1);
    //    gPad->SetLogy();
    gPad->SetGrid();
    gpower2_dt_ch[i]->SetLineColor(colors[i]);
    gpower2_dt_ch[i]->SetMarkerColor(colors[i]);

    //gpower2_dt_ch[i]->SetTitleSize(0.08);
    //gpower2_dt_ch[i]->GetYaxis()->SetLabelSize(0.06);
    //gpower2_dt_ch[i]->GetXaxis()->SetLabelSize(0.06);
    //gpower2_dt_ch[i]->GetYaxis()->SetTitleSize(0.04);
    // gpower2_dt_ch[i]->GetXaxis()->SetTitleSize(0.04);

    if ( gpower2_dt_ch[i]->GetN()>0) gpower2_dt_ch[i]->Draw("Ap");
    gPad->Update();
  }
  c3->SaveAs("./power2_per_channel.png");
  c3->Write("c3");
  for (int i=0; i<25; i++) {
    c3->cd(i+1);
    gPad->SetLogy();
  }
  gPad->Update();
  c3->SaveAs("./power2_per_channel_log.png");
  c3->Write("c3_log");
  

  
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.1);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetOptStat();
  TCanvas *c2 = new TCanvas("c2","c2",2400,1600);

  c2->Divide(3,3);
  c2->cd(1);
  h_dt->Draw();
  gPad->SetLogy();
  gPad->SetGrid();
  c2->cd(2);
  TGraph *gfreq=new TGraph();
  for (int i=0 ; i<gtimes->GetN()-1; i++) {
    double dt = gtimes->GetX()[i+1]-gtimes->GetX()[i];
    if (dt>0) 
      gfreq->SetPoint(gfreq->GetN(),gtimes->GetX()[i], 1/dt*1e6);
  }

  // This code block draws the secondary axis for the rate/time figure
  // Get the secondary axis range by temporarily drawing the second TGraph
  if (gfreq->GetN() > 0)  gfreq->Draw("Al");
  gPad->Update();
  float secondary_axis_min = gPad->GetUymin();
  float secondary_axis_max = gPad->GetUymax();
  if (gtimes->GetN()>0)   gtimes->Draw("A*");
  gPad->Update();
  TGaxis *axis = new TGaxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax(),gPad->GetUymax(),secondary_axis_min,secondary_axis_max,510,"+L");
  float scale = (gPad->GetUymax() - gPad->GetUymin())/(secondary_axis_max-secondary_axis_min);
  //  scale = 1;
  for (int ig=0; ig<gfreq->GetN(); ig++){
    gfreq->GetY()[ig] = gfreq->GetY()[ig] * scale - secondary_axis_min +gPad->GetUymin() ;
  }
  gfreq->SetLineColor(2);
  gfreq->SetMarkerColor(2);
  axis->SetLineColor(2);
  axis->SetLabelColor(2);
  axis->SetTitleColor(2);
  axis->SetTitle("Rate [Hz]");
  axis->Draw();
  if (gfreq->GetN()>0)  gfreq->Draw("*l");
  //// End of secondary axis drawing
  gPad->SetGrid();
  c2->cd(3);
  if (gpower_dt->GetN()>0) gpower_dt->Draw("Al");
  c2->cd(4);
  if (gpower2_dt->GetN()>0)  gpower2_dt->Draw("Al");
  c2->cd(5);
  if (gtimes_dt->GetN()>0)   gtimes_dt->Draw("A*");
  gStyle->SetOptStat(1);
  gPad->Update();
  c2->Write("c2");
  //  TImage* img = TImage::Create();
  //img->FromPad(c1);
  //img->WriteImage("./histogram.png");
  c2->SaveAs("./c2.png");
  delete c1;
  delete c_sigma;
  delete c2;
  delete c3;
  
  // delete img;
  myfile->Close();
}



// EOF

