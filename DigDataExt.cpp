// ==============================================================================
// Digitizer Data Extractor Class Implementation
// ==============================================================================
// Peter Szabo
// Created 02/09/2018
// Updated 31/12/2018
// Modified by Hagar on May 2023 - to include time stamps, and to generate monitoring info

// Add per channel : power by time



// Check titles on all plots
// make a complete run

#include "algorithm"
#include <cstdlib>
#include "TMath.h"
#include "DigDataExt.h"

#include <cmath>
#include <string>
#include "DirexenoCorrection.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TObject.h"
#include "TImage.h"
#include "TPaveStats.h"
#include "TDatime.h"
#include "TGaxis.h"
#include "TROOT.h"
#include "TColor.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TGraphErrors.h"
#include <cstring> // for strtok
//#include <iomanip> // for std::quoted
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <limits> // for std::numeric_limits
#include <bitset>
#include "CAENDigitizerType.h" // #include "/home/home/access/Direxdaq/VME/CAENDigitizer-2.7.6/include/CAENDigitizerType.h"
#include "X742DecodeRoutines.h"
#include "X742CorrectionRoutines.h"
#include <sstream>
#include <vector>
#include <iomanip>
using namespace std;



int colors[25] = {
    TColor::GetColor(255, 0, 0),        // Red
//	3,28,29,4,40,41,5,30,42,6,20,43,7,33,44,8,34,46,9,38,47,37,12,38,29};
    TColor::GetColor(255, 165, 0),      // Orange
    TColor::GetColor(255, 255, 200),      // Yellow
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

// Fix gera's plot
// Check and add to file saves


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

//std::string base = base_name<std::string>("some/path/file.ext");


// Read and parse configuration file, set class parameters
int DigDataExt::ParseSetupFile(const std::string SetupFileName)
{

// open setup file
//for (int i=0; i<25; i++) cout<<"color"<<i<<"="<<colors[i]<<endl;
    
  cout << endl <<  "Loading setup parameters from " << SetupFileName << ":" << endl;
  fstream SetupFile(SetupFileName, ios::in);
  if (!SetupFile) { cerr << "*** File opening error. ***" << endl << endl; exit(1); }

  // read text lines
  string line;
  string end_time("");
  string start_time(""); 
  string end_date("");  
   string start_date("");
  
bitset<10> param_set("0000000000");
  while(getline(SetupFile, line, '#'))
    {

//    cout<<line<<endl;
      if (line.length() >= MAX_CFG_STR) { cerr << "*** Configuration file text line too long.  ***" << endl << endl; exit(1); }
      istringstream iss(line);
      string token;
      iss >> token;
//    cout<<"token="<<token<<"*"<<endl;
      if (token == "PMT_NUM") 
	{ 
	  if (param_set[0]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[0] = 1;
	  iss >> PMTNum; 
	  cout << "PMT_NUM = \t" << PMTNum << endl; 
	}
      else if (token== "RUN_START")
	{   
	    iss>>start_date; 
            iss>>start_time;
            run_time_stamp="";
	    //cout<<"start_time="<<start_date<<" "<<start_time<<endl;
         }
      else if (token== "RUN_END")
	{   
	    iss>>end_date; 
            iss>>end_time;
            run_time_stamp="";
	    //cout<<"end_time="<<end_date<<" "<<end_time<<endl;
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
//          cout<<"line = "<<
	  if (param_set[4]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[4] = 1;
	  string mask_str;
	  iss >> mask_str;
	mask_str.erase(	std::remove(mask_str.begin(), mask_str.end(), '\"'), mask_str.end());

//	  iss >> quoted(mask_str);
//mask_str = mask_str.replace("\"", "");

	  CorrMask = (int)bitset<3>(mask_str).to_ulong();
	  cout << "CORR_MASK = \t0d" << CorrMask << " = 0b" << bitset<3>(CorrMask) << "   (bits: |time|sample|cell|)" << endl; 
	}
//      else if (token == "CORR_PATH") 
//	{ 
//	  if (param_set[5]) { cerr << "*** Parameter " << token << " was already set.  ***" << endl << endl; exit(1); } else param_set[5] = 1;
	 // iss >> quoted(CorrFilesPath);
//	  cout << "CORR_PATH = \t" << CorrFilesPath << endl; 
//	}
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
//	    cout<<i<<endl;
	    PMTTitle[i]+=" ";
	    PMTTitle[i]+=PMTNameMap[i];
	  }
//	  cout<<"ok\n";
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
  if (start_time.length()+start_date.length()>0)
            run_time_stamp=start_date+"_"+start_time;
  if (end_time.length()+end_date.length()>0)
            run_time_stamp="\n"+run_time_stamp+"-->"+end_date+"_"+end_time;

  SetupFile.close();
  return 0;
}

// Load raw data, correct and serialize it, and write it to output file

//DigDataExt::DigDataExt(const string CorrFilesPath, const string setup_file_name, const string dig_file_name, const string out_file_name): SetupFileName(setup_file_name), DigFileName(dig_file_name), OutFileName(out_file_name)


DigDataExt::DigDataExt(const string CorrFilesPath, const string setup_file_name, const string dig_file_name, const string out_file_name, int fast): SetupFileName(setup_file_name), DigFileName(dig_file_name), OutFileName(out_file_name)
{
 
 string short_name = out_file_name.substr(out_file_name.find_last_of("/\\") + 1,out_file_name.find_last_of("."));
 cout<<"----"<<short_name<<"----"<<endl;

  TString root_file_name(OutFileName);
  root_file_name = root_file_name+".root";
  TDatime now;
  run_time_stamp = now.AsString();
//  run_id = "123456";

  // load setup parameters
  for (int i=0; i<30; i++) PMTTitle.push_back(Form("Channel %d",i));
  cout<<PMTTitle.size()<<endl;
  cout << "Starting DigDataExt class." << endl;
  ParseSetupFile(SetupFileName);
  // load correction tables from files
  //if (CorrFilesPath.length() > 0 && CorrFilesPath.back() != '/')
  //  {
 //     CorrFilesPath += '/';
//    }
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
  fstream OutDataFile(OutFileName+".DXD", ios::out|ios::app|ios::binary);
  if (!OutDataFile) { cerr << "*** File opening error. ***" << endl << endl; exit(1); }

  // extract, correct and write data to output file
  cout << "Correcting and serializing events to file:" << endl;
  //  long int previous_time_tag = 0;

  TH1F *h_dt = new TH1F("h_dt","Time differences; Time since event0 [usec]; Entries ",1000,0,1e4);
  TH1F *h_ch_v[PMTNum+ MAX_X742_GROUP_SIZE];
  TH1F *h_ch_v_w_base[PMTNum+ MAX_X742_GROUP_SIZE];
  TH1F *h_ch_sigma[PMTNum+ MAX_X742_GROUP_SIZE];
  TH1F *h_ch_wf_bins[PMTNum+ MAX_X742_GROUP_SIZE];
  TH2F *h2_ch_v[PMTNum+ MAX_X742_GROUP_SIZE];
  TH2F *h2_ch_wf_bins[PMTNum+ MAX_X742_GROUP_SIZE];
  TH1F *h1_baseline_f_ch[PMTNum+ MAX_X742_GROUP_SIZE];
  TH1F *h1_baseline_i_ch[PMTNum+ MAX_X742_GROUP_SIZE];

  // Holds snapshots of waveforms:
  int Nsamples = 5;
  TGraph *gwf_sample[Nsamples][PMTNum+ MAX_X742_GROUP_SIZE];
  TGraph *gtimes=new TGraph();  gtimes->GetXaxis()->SetTitle("Time since event 0 [usec]");  gtimes->GetYaxis()->SetTitle("Event number");
  TGraph *gtimes_dt=new TGraph();   gtimes_dt->GetXaxis()->SetTitle("Time since event 0 [usec]");  gtimes_dt->GetYaxis()->SetTitle("dt from previous  event [usec]");
  TGraph *gpower_dt=new TGraph();   gpower_dt->GetXaxis()->SetTitle("Time since event 0 [usec]");  gpower_dt->GetYaxis()->SetTitle("Total voltage [mv]");
  TGraph *gpower2_dt=new TGraph();   gpower2_dt->GetXaxis()->SetTitle("Time since event 0 [usec]");   gpower2_dt->GetYaxis()->SetTitle("Total power [~mv^2]");
  TGraph *gpower2_dt_ch[PMTNum+ MAX_X742_GROUP_SIZE];
  TGraphErrors *g_baseline_i_ch[PMTNum+ MAX_X742_GROUP_SIZE];
  TGraphErrors *g_baseline_f_ch[PMTNum+ MAX_X742_GROUP_SIZE];
  cout<<"created histograms "<<PMTNum<<endl;
  
  //  NumOfEvents=10000;
  int Nmax = NumOfEvents; //NumOfEvents/3;// /30;
  //Nmax = 1000;
  //  long int first_time_tag = -999;
  long int roundTime=pow(2,30);
  //long int roundTime_usec=roundTime*8.5/1000;
  CAEN_DGTZ_X742_EVENT_t *Event;
  char *EventPtr;
//  DirexenoCorrection *dxc = new DirexenoCorrection("./210223_BL.npz", TimeSamples);
  long int preTime = 0;
  long int preTime_usec = 0;
  long int T_first = -999;
  int tloop = -1;
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
	    //	    cout<<"tloop="<<tloop<<endl;
	  }
	}
	if (T_first == -999) T_first = TimeTag;      
	TimeTag = TimeTag +  tloop*roundTime;
	preTime = TimeTag;
      }
  }
  double_t T_last_usec = preTime*8.5/1000;
  double_t T_first_usec = T_first*8.5/1000;
  
  cout<<"Tfirst = "<<(T_first_usec)<<"Tlast = "<<(T_last_usec)<<"  dt="<<(T_last_usec - T_first_usec)<<" n  = "<<tloop<<endl;;
  int nbins = 1e-6*(T_last_usec - T_first_usec)*5 ; // 2d histogram bin size is 0.2 sec
  if (nbins<5) nbins=10;
  if (nbins>200) nbins=200;
  cout<<T_last_usec-T_first_usec<<endl;
  cout<<1e-6*(T_last_usec-T_first_usec)<<endl;
  
    
  cout<<" nbins= "<<nbins<<"  "<<PMTNum<<endl;
  for (int ch=0; ch<PMTNum+ 0*MAX_X742_GROUP_SIZE;ch++)
    { //    histogram->SetTitle(TString::Format("Histogram: Integer=%d, String=%s", intValue, strValue.c_str()).Data());

      h_ch_v[ch]=    new TH1F(TString::Format("h_ch_v%d",ch).Data(),TString::Format("%s:Voltages wo baseline;counts ADC [mVolts]; Entries",PMTTitle[ch].c_str()).Data(),6096,-2000,4096);
      h_ch_v_w_base[ch]=    new TH1F(TString::Format("h_ch_v_w_base%d",ch).Data(),TString::Format("%s:Voltages w baseline;counts ADC [mVolts]; Entries",PMTTitle[ch].c_str()).Data(),4096,0,4096);
      h_ch_sigma[ch]=new TH1F(TString::Format("h_sigma_%s",PMTTitle[ch].c_str()),TString::Format("%s a:Consecutive sigma; consecutive sigmas, Entries",PMTTitle[ch].c_str()).Data(),10,0,11);
      h_ch_wf_bins[ch]= new TH1F(TString::Format("ch%d_wf",ch).Data(),TString::Format("%s:all bins; [bin]; Entries",PMTTitle[ch].c_str()).Data(),(TimeSamples -20), 0, (TimeSamples-20));
      g_baseline_i_ch[ch] = new TGraphErrors();
      g_baseline_f_ch[ch] = new TGraphErrors();
      gpower2_dt_ch[ch]=new TGraph();
      gpower2_dt_ch[ch]->SetTitle(Form(" - %s",PMTTitle[ch].c_str()));       gpower2_dt_ch[ch]->GetXaxis()->SetTitle("Time since event 0 [usec]");       gpower2_dt_ch[ch]->GetYaxis()->SetTitle("Total power [~mv^2]");
      for (int cn=0; cn<Nsamples; cn++) {
	gwf_sample[cn][ch]=new TGraph(); gwf_sample[cn][ch]->SetTitle(Form("WF sample %d - %s",cn,PMTTitle[ch].c_str()));      
      }	
      //     h2_ch_v[ch]=new TH2F(Form("ch%d_2d",ch),Form("ADC distribution ch%d by time; TimeSince event 0 [usec]; Count [mVolts]",ch),nbins,T_first_usec-T_first_usec,T_last_usec-T_first_usec,4096,0,4096);
      h2_ch_v[ch]=new TH2F(Form("ch%d_2d",ch),Form("ADC distribution ch%d by time; TimeSince event 0 [usec]; Count [mVolts]",ch),nbins,T_first_usec-T_first_usec,T_last_usec-T_first_usec,1e3,0,3000*3000);
      h2_ch_wf_bins[ch] = new TH2F(Form("ch%d_2d_wf",ch),Form("WF distribution ch%d by time; TimeSince event 0 [usec];  [bin]",ch),nbins,T_first_usec-T_first_usec,T_last_usec-T_first_usec,(TimeSamples-20),0,(TimeSamples-20));
      h2_ch_v[ch]->SetTitle(Form("Power distribution ch%s by time",PMTTitle[ch].c_str()));
      h2_ch_v[ch]->GetXaxis()->SetTitle("Time since event 0 [usec]");
      h2_ch_v[ch]->GetYaxis()->SetTitle("sum of v^2");
      h1_baseline_i_ch[ch] =  new TH1F(Form("ch%d_baseline_i",ch),Form("baseline_i ch%d by time; TimeSince event 0 [usec];   baseline [mV]",ch),nbins,T_first_usec-T_first_usec,T_last_usec-T_first_usec);
      h1_baseline_f_ch[ch] =  new TH1F(Form("ch%d_baseline_f",ch),Form("baseline_f ch%d by time; TimeSince event 0 [usec];   baseline [mV]",ch),nbins,T_first_usec-T_first_usec,T_last_usec-T_first_usec);
    }
  tloop=-1;
  TString wf_title[5]={"sample WF A","sample WF B","sample WF C","sample WF D","sample WF E"};

  ///// Main loop begins
  for (int e = 0; e < Nmax; e++)
    {
      Float_t tot_power = 0;
      Float_t tot_power2 = 0;
      EventPtr = NULL;
      double   start_baseline_mean;//[PMTNum+ MAX_X742_GROUP_SIZE];
      double   end_baseline_mean;//[PMTNum+ MAX_X742_GROUP_SIZE];
      double   start_baseline_rms;//[PMTNum+ MAX_X742_GROUP_SIZE];
      double   end_baseline_rms;//[PMTNum+ MAX_X742_GROUP_SIZE];

      GetEventPtr(DataBuffer, DataBufferSize, e, &EventPtr);
      Event = NULL;
      X742_DecodeEvent(EventPtr, (void **)&Event);  // <===== memory leak is here
      // correct data
      for (int j = 0; j < MAX_X742_GROUP_SIZE; j++)
	{  
//cout<<"Before = "<<Event->DataGroup[j].DataChannel[7][634]<<endl; 
	  ApplyDataCorrection(CorrTable[j], SampFreq, CorrMask, &Event->DataGroup[j]);
//cout<<"After = "<<Event->DataGroup[j].DataChannel[7][634]<<endl; 

	}
      long int TimeTag = (long int)Event->DataGroup[0].TriggerTimeTag;
      long int TimeTag_usec = (long int)Event->DataGroup[0].TriggerTimeTag*8.5/1000;
      for (int g = 1; g<4; g++) 
	if (TimeTag !=  (long int)Event->DataGroup[g].TriggerTimeTag)
	  cout<<"*************     Time tags of groups are different   ******"<<endl;

      //if (preTime!=-999) {
      if (TimeTag+tloop*roundTime < preTime) tloop++;
	//}
      if (T_first == -999) T_first = TimeTag;      
      TimeTag = TimeTag + tloop*roundTime;
      TimeTag_usec = TimeTag * 8.5/1000;
      long int TimeTag0_usec = TimeTag_usec - T_first_usec;
      if (e>0)
	{
	  h_dt->Fill(TimeTag_usec - preTime_usec);
	  //	      h_dt->Fill(preTime_usec - TimeTag_usec);
	  gtimes_dt->SetPoint(gtimes_dt->GetN(),TimeTag0_usec, TimeTag_usec - preTime_usec);
	  // gtimes_dt->SetPoint(gtimes_dt->GetN(),event_dt0, event_dt);
	  gtimes->SetPoint(gtimes->GetN(),TimeTag_usec - T_first_usec, e);
	  //	      gtimes->SetPoint(gtimes->GetN(),event_dt0, e);
	      
	}
      //}
      //previous_time_tag = TimeTag;
      preTime = TimeTag;
      preTime_usec = preTime*8.5/1000;

      // ch is the number of pmt.   c,g is the number of the channel on digitizer
      // keep
      
      for (int ch = 0; ch < (PMTNum + -0*MAX_X742_GROUP_SIZE); ch++)	{
	int g, c;
	
	if (ch < PMTNum) { // PMT channel 
	  g = PMTChMap[ch] / (int)(MAX_X742_CHANNEL_SIZE - 1); // MAX_X742_CHANNEL_SIZE = 9 due to the fast trigger channels (see CAENDigitizerType.h)
	  c = PMTChMap[ch] % (int)(MAX_X742_CHANNEL_SIZE - 1);
	}
	else { // fast trigger ch annel
	  g = ch - PMTNum;
	  c = MAX_X742_CHANNEL_SIZE - 1;
	}
	// check data length integrity (all data vectors should be TimeSamples in length)
	if (TimeSamples != Event->DataGroup[g].ChSize[c]) { cerr << "*** Event " << e << " PMT/TR " << ch << " data length is not " << TimeSamples << " ***" << endl << endl; exit(1); }

	float StartIndex = (float)Event->DataGroup[g].StartIndexCell;
	
	start_baseline_mean=TMath::Mean(50,&Event->DataGroup[g].DataChannel[c][0]) ;
	end_baseline_mean=TMath::Mean(50,&Event->DataGroup[g].DataChannel[c][TimeSamples-20-50]) ;
	start_baseline_rms=TMath::RMS(50,&Event->DataGroup[g].DataChannel[c][0]) ;
	end_baseline_rms=TMath::RMS(50,&Event->DataGroup[g].DataChannel[c][TimeSamples-20-50]) ;
	g_baseline_i_ch[ch]->SetPoint(g_baseline_i_ch[ch]->GetN(),TimeTag0_usec, start_baseline_mean);
	g_baseline_i_ch[ch]->SetPointError(g_baseline_i_ch[ch]->GetN()-1,0,start_baseline_rms);
	g_baseline_f_ch[ch]->SetPoint(g_baseline_f_ch[ch]->GetN(),TimeTag0_usec, end_baseline_mean);
	g_baseline_f_ch[ch]->SetPointError(g_baseline_f_ch[ch]->GetN()-1,0,end_baseline_rms);
	//	cout<<"baseline_i:"<<g_baseline_i_ch[ch]->GetX()[nn]<<"\t"<<g_baseline_i_ch[ch]->GetY()[nn]<<g_baseline_i_ch[ch]->GetErrorY(nn)<<endl;
	// first write PMT channel data to file by PMT order, and then write the four fast-trigger channels
	// save data to file
	OutDataFile.write((char*)&TimeTag, sizeof(float));
	OutDataFile.write((char*)&StartIndex, sizeof(float));
	OutDataFile.write((char*)Event->DataGroup[g].DataChannel[c], TimeSamples * sizeof(float));   

	float tot_power2_ch=0;
	TGraph *gtemp = new TGraph();

	for (int b=0; b<(int) TimeSamples-20; b++){
	    gtemp->SetPoint(gtemp->GetN(),b,Event->DataGroup[g].DataChannel[c][b]);
	    h_ch_v[ch]->Fill(Event->DataGroup[g].DataChannel[c][b] - start_baseline_mean);
	    h_ch_v_w_base[ch]->Fill(Event->DataGroup[g].DataChannel[c][b]);
	    h2_ch_v[ch]->Fill(TimeTag0_usec, pow(Event->DataGroup[g].DataChannel[c][b]-start_baseline_mean,2) );
	    if (b>2 && b<(int) TimeSamples-20) h2_ch_wf_bins[ch]->Fill(TimeTag0_usec, b, Event->DataGroup[g].DataChannel[c][b]-start_baseline_mean );
	    if (b>2 && b<(int) TimeSamples-20) h_ch_wf_bins[ch]->Fill(b,Event->DataGroup[g].DataChannel[c][b]-start_baseline_mean );    
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
	  
	  delete gtemp; // hyl
	  int index = -999;
	  int sig1=6;
	  int sig2=3;

	  if (oks[sig1]>6) {
	    index = 4;
	    gwf_sample[index][ch]->SetTitle(Form("More than %d sigmas, WF %d - %s  (mn=%.2f,stv=%.2f)",sig1,e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	    wf_title[index]=Form("Sample WF, over %d sigmas",sig1);
	  }
	  if (oks[sig2]>6  && index==-999) {
	    index = 3;
	    gwf_sample[index][ch]->SetTitle(Form("More than %d sigmas, WF %d - %s  (mn=%.2f,stv=%.2f)",sig2,e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	    wf_title[index]=Form("Sample WF, over %d sigmas",sig2);

	  }
	  int n_consecutive=6;
	  for (int iok=0; iok<10; iok++) if (oks[iok]>n_consecutive) h_ch_sigma[ch]->Fill(iok);
	  h_ch_sigma[ch]->SetTitle(Form("at least %d consecutive bins are x sig above baseline, ch%d",n_consecutive,ch));
	
	  if (e==0) {
	    index=0;
	    gwf_sample[index][ch]->SetTitle(Form("WF %d - %s  (mn=%.2f,stv=%.2f)",e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	    wf_title[index]=Form("Sample WF, event %d",e);

	  }
	  if (e==Nmax/2 ) {
	    index=1;
	    gwf_sample[index][ch]->SetTitle(Form("WF %d - %s  (mn=%.2f,stv=%.2f)",e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	    wf_title[index]=Form("Sample WF, event %d",e);

	  }
	  if (e==Nmax-1) {
	    index=2;
	    gwf_sample[index][ch]->SetTitle(Form("WF %d - %s  (mn=%.2f,stv=%.2f)",e,PMTTitle[ch].c_str(),h_mean,h_stdev));
	    wf_title[index]=Form("Sample WF, event %d",e);
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
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      
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

   //file myfile->WriteObject(h_dt, "h_dt");
     //file gtimes->Write("gtimes");
     //file gtimes_dt->Write("gtimes_dt");
     //file gpower_dt->Write("gpower_dt");
     //file gpower2_dt->Write("gpower2_dt");
  

  float x_range_width = 300; //for graphics. Will try to keep xaxis to this range

  // Summarize individual channels monitoring information: Fits, write
  //double ch_mean[PMTNum + MAX_X742_GROUP_SIZE];
  double ch_stdev[PMTNum + MAX_X742_GROUP_SIZE];
  // int voltage_lastbin[25];
  //int voltage_firstbin[25];

  for (int ch=0; ch<PMTNum + 0*MAX_X742_GROUP_SIZE; ch++){ // skip the first channel (trigger)
    float b_first=0;
    float b_last=0;
    int binmax = h_ch_v_w_base[ch]->GetMaximumBin();
    for (int b=0; b<h_ch_v_w_base[ch]->GetNbinsX(); b++) {
      float v = h_ch_v_w_base[ch]->GetBinContent(b);
      if (b_first==0 && v>0) b_first=h_ch_v_w_base[ch]->GetBinCenter(b-1);
      if (v>0) b_last=h_ch_v_w_base[ch]->GetBinCenter(b+1);;
    }
    //    voltage_lastbin[ch] = b_last;
    //voltage_firstbin[ch] = b_first;
    if (b_last - b_first < x_range_width) {
      float delta =  x_range_width - (b_last - b_first);
      h_ch_v_w_base[ch]->GetXaxis()->SetRangeUser(b_first-delta/2, b_last+delta/2);
    }
    else
      {
	h_ch_v_w_base[ch]->GetXaxis()->SetRangeUser(b_first-30, b_last+30);
      }
    h_ch_v_w_base[ch]->Fit("gaus", "Q","",  h_ch_v_w_base[ch]->GetXaxis()->GetBinCenter(binmax-50),  h_ch_v_w_base[ch]->GetXaxis()->GetBinCenter(binmax+50));
    // cout<<binmax<<"==binmax"<<endl;
    TF1 *fit1 = h_ch_v_w_base[ch]->GetFunction("gaus");
    //int fitok=0;
    // ch_mean[ch] = h_ch_v[ch]->GetMean();
    ch_stdev[ch] = 20;
    if (fit1) {
      //fitok=1;
      fit1->SetLineStyle(2);
      //ch_mean[ch] = fit->GetParameter(1);
      ch_stdev[ch] = fit1->GetParameter(2);
   //file       h_ch_v[ch]->Write();
   //file       gpower2_dt_ch[ch]->Write(Form("gpower2_dt_%d",ch));
      //gwf_sample1[ch]->Write(Form("gwf_sample1_%d",ch));
      //gwf_sample2[ch]->Write(Form("gwf_sample2_%d",ch));
      //gwf_sample3[ch]->Write(Form("gwf_sample3_%d",ch));
      // gwf_sample4[ch]->Write(Form("gwf_sample4_%d",ch));
    }
  }
  
  for (int ch=0; ch<PMTNum + 0*MAX_X742_GROUP_SIZE; ch++){ // skip the first channel (trigger)
    float b_first=0;
    float b_last=0;
    int binmax = h_ch_v[ch]->GetMaximumBin();
    for (int b=0; b<h_ch_v[ch]->GetNbinsX(); b++) {
      float v = h_ch_v[ch]->GetBinContent(b);
      if (b_first==0 && v>0) b_first=h_ch_v[ch]->GetBinCenter(b-1);
      if (v>0) b_last=h_ch_v[ch]->GetBinCenter(b+1);;
    }
    //    voltage_lastbin[ch] = b_last;
    //voltage_firstbin[ch] = b_first;
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
    TF1 *fit2 = h_ch_v[ch]->GetFunction("gaus");
    //int fitok=0;
    // ch_mean[ch] = h_ch_v[ch]->GetMean();
    ch_stdev[ch] = 20;
    if (fit2) {
      //fitok=1;
      fit2->SetLineStyle(2);
      //ch_mean[ch] = fit->GetParameter(1);
      ch_stdev[ch] = fit2->GetParameter(2);
   //file       h_ch_v[ch]->Write();
   //file       gpower2_dt_ch[ch]->Write(Form("gpower2_dt_%d",ch));
      //gwf_sample1[ch]->Write(Form("gwf_sample1_%d",ch));
      //gwf_sample2[ch]->Write(Form("gwf_sample2_%d",ch));
      //gwf_sample3[ch]->Write(Form("gwf_sample3_%d",ch));
      // gwf_sample4[ch]->Write(Form("gwf_sample4_%d",ch));
    }
  }





  // Fill 2d histogram
  // for (int i=0; i<25; i++)     h2_ch_v[i]->GetYaxis()->SetRange(voltage_firstbin[i]-20,voltage_lastbin[i]+20);
  
  for (int i=0; i<25; i++)   {
    int by = -999;
    float vy=0;
    //   cout<<"bins: "<< h2_ch_v[i]->GetNbinsY()<<"   and x:"<< h2_ch_v[i]->GetNbinsX()<<endl;
    for ( by = h2_ch_v[i]->GetNbinsY(); by>0; by--) {
      if (vy>0) break;
      for (int bx=1; bx< h2_ch_v[i]->GetNbinsX();bx++) {
	float v = h2_ch_v[i]->GetBinContent(bx,by);	
	if (v>0) {
	  vy = h2_ch_v[i]->GetYaxis()->GetBinCenter(by);
	  break;
	}
      }
    }
    h2_ch_v[i]->GetYaxis()->SetRange(0,by);
    //    cout<<" Range = "<<by<<"   v="<<vy<<endl;
  }
  
  drawObjects((TObject**) h2_ch_v, root_file_name, OutFileName+"_power_2d_a", "Overall power by channel, over time",run_time_stamp,0,0,1);
  for (int i=0; i<25; i++)  {
    Int_t lastBinY = h2_ch_v[i]->GetYaxis()->FindBin( 10*pow(ch_stdev[i],2));
    Int_t firstBinY = h2_ch_v[i]->GetYaxis()->FindBin( 1);
    h2_ch_v[i]->GetYaxis()->SetRange(firstBinY,lastBinY);
  }
  //  drawObjects((TObject**) h2_ch_v, "power_2d_b", "Power over time",0,0,1);
	 
  drawObjects((TObject**)  h2_ch_wf_bins,root_file_name, OutFileName+ "_ch_wf_bin_2d", "Bins distribution  over time",run_time_stamp, 0,0,1);

  drawObjects((TObject**)  h_ch_wf_bins, root_file_name, OutFileName+"_ch_wf_bin_1d", "Overall Bins distributions",run_time_stamp, 0,0,1);		

  drawObjects((TObject**) h_ch_v,root_file_name, OutFileName+"_voltages","Voltages distribution baseline removed",run_time_stamp,1,1,0);

  drawObjects((TObject**) h_ch_v_w_base,root_file_name, OutFileName+"_voltages_w_base","Voltages distribution w baseline",run_time_stamp,1,1,0);

  for (int cn=0; cn<Nsamples; cn++)
    drawObjects((TObject**) gwf_sample[cn],root_file_name, OutFileName+Form("_WF_sample_%d",cn),wf_title[cn],run_time_stamp,0,0,0);

  drawObjects((TObject**) h_ch_sigma,root_file_name, OutFileName+"_sigmas","Number of over sigma events", run_time_stamp,1,0,0);

  drawObjects((TObject**) gpower2_dt_ch,root_file_name,OutFileName+"_power2","power square", run_time_stamp,1,0,0);

  // draw2Objects((TObject**)g_baseline_i_ch,g_baseline_f_ch,"baseline_i",0,0,0);

  

  for (int ch=0; ch<25; ch++) {//kkk
    int prebin=0;
    int newIndex=0; 
    std::vector<Double_t> meanValues_i;
    std::vector<Double_t> rmsValues_i;
    std::vector<Double_t> meanValues_f;
    std::vector<Double_t> rmsValues_f;
    float min = -999;
    float max = -999;
    for (int i=0; i< g_baseline_i_ch[ch]->GetN(); i++) {
      double x = g_baseline_i_ch[ch]->GetX()[i];
      int thisbin = h1_baseline_i_ch[ch]->FindBin(x);
      if (thisbin!=prebin) {
	Double_t combinedMean_i = 0.0;
	Double_t combinedRMS_i = 0.0;
//	cout<<"mens="<<meanValues_i.size()<<" "<<rmsValues_i.size()<<endl;
	if (meanValues_i.size()>0  && rmsValues_i.size()>0)
		CombineMeanAndRMS(combinedMean_i, combinedRMS_i, meanValues_i, rmsValues_i, i-newIndex);
//	printf ("from %d to %d = %f %f\n",newIndex, i-1,combinedMean_i, combinedRMS_i);
	if (combinedMean_i>0 && prebin>1) {
//	  printf ("\t bin %d = %f %f\n",prebin,combinedMean_i, combinedRMS_i);
	  h1_baseline_i_ch[ch]->SetBinContent(prebin, combinedMean_i);
	  h1_baseline_i_ch[ch]->SetBinError(prebin, combinedRMS_i);
	  if (min == -999 || combinedMean_i-combinedRMS_i < min) min = combinedMean_i-combinedRMS_i;
	  if (max == -999 || combinedMean_i+combinedRMS_i > min) max = combinedMean_i+combinedRMS_i;
	}
	Double_t combinedMean_f = 0.0;
	Double_t combinedRMS_f = 0.0;

	CombineMeanAndRMS(combinedMean_f, combinedRMS_f, meanValues_f, rmsValues_f, i-newIndex);
	//	printf ("from %d to %d = %f %f\n",newIndex, i-1,combinedMean, combinedRMS);
	if (combinedMean_f>0 && prebin>0) {
	  h1_baseline_f_ch[ch]->SetBinContent(prebin, combinedMean_f);
	  h1_baseline_f_ch[ch]->SetBinError(prebin, combinedRMS_f);
	  if (min == -999 || combinedMean_f-combinedRMS_f < min) min = combinedMean_f-combinedRMS_f;
	  if (max == -999 || combinedMean_f+combinedRMS_f > min) max = combinedMean_f+combinedRMS_f;
	}

	h1_baseline_i_ch[ch]->GetYaxis()->SetRangeUser(min, max);
	h1_baseline_f_ch[ch]->GetYaxis()->SetRangeUser(min, max);

	newIndex = i;
	prebin=thisbin;
	rmsValues_i.clear();
	meanValues_i.clear();
	rmsValues_f.clear();
	meanValues_f.clear();
      }
      else {
	meanValues_i.push_back( g_baseline_i_ch[ch]->GetY()[i]);
	rmsValues_i.push_back( g_baseline_i_ch[ch]->GetErrorY(i));
	meanValues_f.push_back( g_baseline_f_ch[ch]->GetY()[i]);
	rmsValues_f.push_back( g_baseline_f_ch[ch]->GetErrorY(i));

      }
    }
   //file     h1_baseline_i_ch[ch]->Write(Form("h1_baseline_i_%d",ch));
     //file   h1_baseline_f_ch[ch]->Write(Form("h1_baseline_f_%d",ch));
    
  }

  draw2Objects((TObject**) h1_baseline_i_ch, (TObject**) h1_baseline_f_ch,root_file_name, OutFileName+"_h_baseline_i","Direxeno Baseline ",run_time_stamp,0,0,0);
  //  drawObjects((TObject**)	h1_baseline_f_ch,"h_baseline_f",0,0,0);

  /*    double mean = g_baseline_i_ch->GetY()[i];
    double rms = g_baseline_i_ch->GetErrorY(i);
    
    if (thisbin == prebin) {
      vals.push_back(
*/


  // Draw timing histograms:
  
  TCanvas *c2 = new TCanvas("c2","c2",2400,1600);

  TPad title_pad("title_pad","title_pad",0.01,0.95,0.8,0.99);
  title_pad.Draw();
  TPad date_pad("date_pad","date_pad",0.8,0.95,0.99,0.99);
  date_pad.Draw();
  TPad bottom_pad("bottom_pad", "bottom_pad", 0.01, 0.01, 0.99, 0.94);
  bottom_pad.Draw();

  date_pad.cd();
  TPaveText *pt0 = new TPaveText(0.01,0.01,.99,.99);
  pt0->AddText(run_time_stamp);
  pt0->AddText(Form("ID: %s",run_id.Data()));
  pt0->Draw();
  
  title_pad.cd();
  TPaveText *pt1 = new TPaveText(0.01,0.01,0.99,0.99);
  pt1->AddText("Run timing details");
  pt1->Draw();

  gStyle->SetOptStat(0);

  // canvas->cd(); // This line is REALLY important otherwise you'll draw a subpad in the top pad :-(

  bottom_pad.cd();
  bottom_pad.Divide(2,3);//,0.00001,0.00001);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.1);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetTitleOffset(0.01,"xyz");
    //    gPad->SetTitleOffset(0.01,"xyz");
  bottom_pad.cd(1);
    //c->cd(i+1);
  gPad->SetLogy();
  gPad->SetGrid();
  h_dt->Draw();
  bottom_pad.cd(2);
  TGraph *gfreq=new TGraph();
  gfreq->SetTitle("Trigger frequency; Time since event 0 [usec]; Frequency [Hz]");
									   
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
  TGaxis *axis = new TGaxis(gPad->GetUxmax()*0.95,gPad->GetUymin(),gPad->GetUxmax()*0.95,gPad->GetUymax(),secondary_axis_min,secondary_axis_max,510,"+L");
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
  
  bottom_pad.cd(3);
  if (gpower_dt->GetN()>0) gpower_dt->Draw("Al");
  gPad->SetGrid();
  gPad->Update();

  bottom_pad.cd(4);
  if (gpower2_dt->GetN()>0)  gpower2_dt->Draw("Al");
  gPad->SetGrid();
  gPad->Update();

  bottom_pad.cd(5);
  if (gtimes_dt->GetN()>0)   gtimes_dt->Draw("A*");
  gStyle->SetOptStat(1);
  gPad->SetGrid();
  gPad->Update();

  bottom_pad.cd(6);
  gfreq->Draw("Al*");
  gPad->SetGrid();
  gPad->Update();
  
  //file   c2->Write("c2");
//  cout<<"wrote c2"<<endl;
  //  TImage* img = TImage::Create();
  //img->FromPad(c1);
  //img->WriteImage("./histogram.png");
  TString  new_name(OutFileName);
  new_name =  new_name.ReplaceAll(".DXD","");
  //file  

//  myfile = TFile::Open("file.root","update");
// c2->Write("c2"); 
  //myfile->Close();

  c2->SaveAs(Form("%s_crunTiming.pdf",new_name.Data()));
  c2->SaveAs(Form("%s_crunTiming.png",new_name.Data()));



  TFile* myfile = TFile::Open(root_file_name,"recreate");
  for (int i=0; i<25; i++) { 
  h2_ch_v[i]->Write();
  h2_ch_wf_bins[i]->Write();
  h_ch_v[i]->Write();
  h_ch_v_w_base[i]->Write();
  h_ch_sigma[i]->Write();
  h1_baseline_i_ch[i]->Write();
//  g_baseline_i_ch[i]->Write();
  h1_baseline_f_ch[i]->Write();
  gpower2_dt_ch[i]->Write(Form("gPower_ch%d",i));
 // g_baseline_i_ch[i]->Write();

  for (int j=0; j<5; j++)
	gwf_sample[j][i]->Write(Form("gwf_ch%d_set%d",i,j));

}

gfreq->Write();
gpower_dt->Write();
gpower2_dt->Write();
gtimes_dt->Write();
gtimes->Write();
myfile->Close();


//Draw moshe

// myfile = TFile::Open(root_file_name,"UPDATE"); // First time writing to a file
  /*
  pt0->Write();
  pt1->Write();
  h_dt->Write();
  gfreq->Write("gfreq");
  gtimes->Write("gtimes");
  gpower_dt->Write();
  gpower2_dt->Write();
  gtimes_dt->Write(); 
 */  
  //cout<<"saved c2"<<endl;
  // delete img;
  // myfile->Close();


}


/*

void drawObjects(TObject** objects, TString name, int logy, int fits, int logz)
{
  TCanvas * c = new TCanvas(Form("c_%s",name),name,3600,3600);
  c->Divide(5,5);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(111);
  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.01);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetTitleOffset(0.01,"xyz");


  for (int i = 0; i < 25; i++) {
    c->cd(i+1);
    gPad->SetLogy(logy);
    gPad->SetLogz(logz);
    
    gPad->SetGrid();
    TObject* obj = objects[i];        
    
    if (obj->InheritsFrom(TGraphErrors::Class())) {
            TGraphErrors* graph = static_cast<TGraphErrors*>(obj);
	    graph->SetLineColor(colors[i]);
	    graph->SetMarkerColor(colors[i]);
	    if (graph->GetN()>0) 
	      graph->Draw("APL");
	    //	    for (int i=0; i<graph->GetN(); i++ ) cout<<graph->GetX()[i]<<"\t"<<graph->GetY()[i]<<"\t"<<graph->GetErrorY(i)<<endl;
    }
    else if (obj->InheritsFrom(TGraph::Class())) {
            TGraph* graph = static_cast<TGraph*>(obj);
	    graph->SetLineColor(colors[i]);
	    graph->SetMarkerColor(colors[i]);
	    if (graph->GetN()>0) 
	      graph->Draw("APL");
    }
    else if (obj->InheritsFrom(TH1F::Class())) {
            TH1F* histogram = static_cast<TH1F*>(obj);
	    histogram->SetLineWidth(2);
	    histogram->SetLineColor(colors[i]);
	    histogram->SetMarkerColor(colors[i]);
	    histogram->SetTitleSize(0.08);
	    histogram->GetYaxis()->SetLabelSize(0.06);
	    histogram->GetXaxis()->SetLabelSize(0.06);
	    histogram->GetYaxis()->SetTitleSize(0.04);
	    histogram->GetXaxis()->SetTitleSize(0.04);

	    histogram->Draw();
	    histogram->Write(Form("h_ch%d_"+name,i));

    }
    else if (obj->InheritsFrom(TH2F::Class())) {
            TH2F* histogram = static_cast<TH2F*>(obj);
	    histogram->Draw("colz");
    }
    else {
      cout << "Error: Unsupported object type at index " << i << "!" << endl;
      continue;
    }
    if (fits) {
      gPad->Update();
      gStyle->SetOptStat(0);
      gStyle->SetOptFit(111);
      TPaveStats *stats = (TPaveStats*) gPad->GetPrimitive("stats");
      if (stats) {
	stats->SetFillStyle(0);
	stats->SetX1NDC(0.1); //new x start position
	stats->SetX2NDC(0.5); //new x end position
	stats->SetY1NDC(0.6); //new x start position
	stats->SetY2NDC(0.9); //new x end position
	//    stats->SetY1NDC(0.5); //new x start position
	//stats->SetY2NDC(1); //new x end position

      }
      
    }

    gPad->Update();

  }
  gPad->Update();
    c->SaveAs(name+".pdf");
    c->Write("c"+name);
    delete c;


}*/


void drawObjects(TObject** objects, TString root_file_name, TString name, TString title,  TString timestring, int logy, int fits, int logz, int PMTNum)
{
  TCanvas * c = new TCanvas("c_"+name,name,3600,3600);
  //  TDatime now;
  //TString timestring = now.AsString();

  TPad title_pad("title_pad","title_pad",0.01,0.95,0.8,0.99);
  title_pad.Draw();
  TPad date_pad("date_pad","date_pad",0.8,0.95,0.99,0.99);
  date_pad.Draw();
  TPad bottom_pad("bottom_pad", "bottom_pad", 0.01, 0.01, 0.99, 0.94);
  bottom_pad.Draw();

  date_pad.cd();
  TPaveText *pt0 = new TPaveText(0.01,0.01,.99,.99);
  pt0->AddText(timestring);
  pt0->Draw();
  
  title_pad.cd();
  TPaveText *pt1 = new TPaveText(0.01,0.01,0.99,0.99);
  pt1->AddText(title);
  pt1->Draw();

  gStyle->SetOptStat(0);

  // canvas->cd(); // This line is REALLY important otherwise you'll draw a subpad in the top pad :-(

  bottom_pad.cd();
  bottom_pad.Divide(5,5,0.00001,0.00001);
    
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(111);
  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.01);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetTitleOffset(0.01,"xyz");
    //    gPad->SetTitleOffset(0.01,"xyz");

//   TFile* myfile = TFile::Open(root_file_name,"UPDATE");
   
  for (int i = 0; i < 25; i++) {
    bottom_pad.cd(i+1);
    //c->cd(i+1);
    gPad->SetLogy(logy);
    gPad->SetLogz(logz);
    gPad->SetBottomMargin(0.1);
    gPad->SetLeftMargin(0.07);
    gPad->SetRightMargin(0.04);
    gPad->SetTopMargin(0.);
    
    gPad->SetGrid();
    TObject* obj = objects[i];        
   // obj->Write();
    if (obj->InheritsFrom(TGraphErrors::Class())) {
            TGraphErrors* graph = static_cast<TGraphErrors*>(obj);
	    graph->SetLineColor(colors[i]);
	    graph->SetMarkerColor(colors[i]);
	    if (graph->GetN()>0) 
	      graph->Draw("APL");
	    //	    for (int i=0; i<graph->GetN(); i++ ) cout<<graph->GetX()[i]<<"\t"<<graph->GetY()[i]<<"\t"<<graph->GetErrorY(i)<<endl;
    }
    else if (obj->InheritsFrom(TGraph::Class())) {
            TGraph* graph = static_cast<TGraph*>(obj);
	    graph->SetLineColor(colors[i]);
	    graph->SetMarkerColor(colors[i]);
	    if (graph->GetN()>0) 
	      graph->Draw("APL");
    }
    else if (obj->InheritsFrom(TH1F::Class())) {
            TH1F* histogram = static_cast<TH1F*>(obj);
	    histogram->SetTitleSize(0.08);
	    histogram->GetYaxis()->SetLabelOffset(-0.07);
	    histogram->GetYaxis()->SetTitleOffset(0.4);
	    histogram->SetLineWidth(2);
	    histogram->SetLineColor(colors[i]);
	    histogram->SetMarkerColor(colors[i]);
	    histogram->SetTitleSize(0.08);
	    histogram->GetYaxis()->SetLabelSize(0.05);
	    histogram->GetXaxis()->SetLabelSize(0.05);
	    histogram->GetYaxis()->SetTitleSize(0.04);
	    histogram->GetXaxis()->SetTitleSize(0.04);
//	    histogram->GetYaxis()->SetMaxDigits(2);
   TGaxis::SetMaxDigits(3);

	    histogram->Draw();
	    gPad->Update();

   //file 	    histogram->Write(Form(name+"h_ch%d_",i));

    }
    else if (obj->InheritsFrom(TH2F::Class())) {
            TH2F* histogram = static_cast<TH2F*>(obj);
	    histogram->SetTitleSize(0.08);
	    histogram->GetYaxis()->SetLabelOffset(-0.07);
	    histogram->GetYaxis()->SetTitleOffset(0.4);
	    histogram->SetLineWidth(2);
	    histogram->SetLineColor(colors[i]);
	    histogram->SetMarkerColor(colors[i]);
	    histogram->SetTitleSize(0.08);
	    histogram->GetYaxis()->SetLabelSize(0.05);
	    histogram->GetXaxis()->SetLabelSize(0.05);
	    histogram->GetYaxis()->SetTitleSize(0.04);
	    histogram->GetXaxis()->SetTitleSize(0.04);
//	    histogram->GetYaxis()->SetMaxDigits(2);
   TGaxis::SetMaxDigits(3);

	    histogram->Draw("colz");
   //file 	    histogram->Write(Form(name+"h_ch%d_",i));

    }
    else {
      cout << "Error: Unsupported object type at index " << i << "!" << endl;
      continue;
    }
    if (fits) {
      gStyle->SetOptStat(0);
      gStyle->SetOptFit(111);
       TPaveStats *stats = (TPaveStats*) gPad->GetPrimitive("stats");
      if (stats) {
	stats->SetFillStyle(0);
	stats->SetX1NDC(0.1); //new x start position
	stats->SetX2NDC(0.5); //new x end position
	stats->SetY1NDC(0.6); //new x start position
	stats->SetY2NDC(0.9); //new x end position
	//    stats->SetY1NDC(0.5); //new x start position
	//stats->SetY2NDC(1); //new x end position

      }
      
    }

    gPad->Update();

  }
	
  c->SaveAs(name+"_c.C");
  //myfile->Close();
  gPad->Update();
  TString  new_name(name);
//  new_name =  new_name.ReplaceAll(".DXD","");
  c->SaveAs(new_name+".pdf");
  c->SaveAs(new_name+".png");
      //file  c->Write("c"+name);
    //    delete c;


} 

/*

 
  TH1F *h=new TH1F("h","h;bla bla ; bla bla",100,1,100);
  TCanvas* canvas = new TCanvas( "SomeName", "SomeTitle", 3600, 2400 );
  
  TPad title_pad("title_pad","title_pad",0.01,0.95,0.8,0.99);
  title_pad.Draw();
  TPad date_pad("date_pad","date_pad",0.8,0.95,0.99,0.99);
  date_pad.Draw();
  TPad bottom_pad("bottom_pad", "bottom_pad", 0.01, 0.01, 0.99, 0.94);
  bottom_pad.Draw();

  date_pad.cd();
  TPaveText *pt0 = new TPaveText(0.01,0.01,.99,.99);
  pt0->AddText(timestring);
  pt0->Draw();
  
  title_pad.cd();
  TPaveText *pt1 = new TPaveText(0.01,0.01,0.99,0.99);
  pt1->AddText("Title bla bla bla bla ");
  pt1->Draw();

  gStyle->SetOptStat(0);

  // canvas->cd(); // This line is REALLY important otherwise you'll draw a subpad in the top pad :-(

  bottom_pad.cd();
  bottom_pad.Divide(5,5,0.00001,0.00001);
  for (int i=1; i<26; i++) {
    bottom_pad.cd(i);
    gPad->SetBottomMargin(0.1);
    gPad->SetLeftMargin(0.07);
    gPad->SetRightMargin(0.04);
    gPad->SetTopMargin(0.);
    //    gPad->SetTitleOffset(0.01,"xyz");
    h->SetTitleSize(0.08);
    h->GetYaxis()->SetTitleSize(0.08);
    h->GetYaxis()->SetLabelSize(0.07);
    h->GetYaxis()->SetLabelOffset(-0.07);
    h->GetYaxis()->SetTitleOffset(0.4);
    h->GetXaxis()->SetTitleSize(0.08);
    h->GetXaxis()->SetLabelSize(0.07);
    h->Draw();

 */

void draw2Objects(TObject** objects1, TObject** objects2,TString root_file_name, TString name, TString title, TString timestring,int logy, int fits, int logz, int PMTNum)
{

  TCanvas * c = new TCanvas(TString::Format("c_%s",name.Data()),name,3600,3600);
  //  TDatime now;
  //TString timestring = now.AsString();

  TPad title_pad("title_pad","title_pad",0.01,0.95,0.8,0.99);
  title_pad.Draw();
  TPad date_pad("date_pad","date_pad",0.8,0.95,0.99,0.99);
  date_pad.Draw();
  TPad bottom_pad("bottom_pad", "bottom_pad", 0.01, 0.01, 0.99, 0.94);
  bottom_pad.Draw();

  date_pad.cd();
  TPaveText *pt0 = new TPaveText(0.01,0.01,.99,.99);
  pt0->AddText(timestring);
  pt0->Draw();
  
  title_pad.cd();
  TPaveText *pt1 = new TPaveText(0.01,0.01,0.99,0.99);
  pt1->AddText(title);
  pt1->Draw();

  gStyle->SetOptStat(0);

  // canvas->cd(); // This line is REALLY important otherwise you'll draw a subpad in the top pad :-(

  bottom_pad.cd();
  bottom_pad.Divide(5,5,0.00001,0.00001);
    
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(111);
  gStyle->SetPadTopMargin(0.01);
  gStyle->SetPadLeftMargin(0.1);
  gStyle->SetPadRightMargin(0.01);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetTitleOffset(0.01,"xyz");
    //    gPad->SetTitleOffset(0.01,"xyz");
  TFile* myfile = TFile::Open(root_file_name,"UPDATE");
   
  for (int i = 0; i < 25; i++) {
    bottom_pad.cd(i+1);
    //c->cd(i+1);
    gPad->SetLogy(logy);
    gPad->SetLogz(logz);
    gPad->SetBottomMargin(0.1);
    gPad->SetLeftMargin(0.07);
    gPad->SetRightMargin(0.04);
    gPad->SetTopMargin(0.);
    
    gPad->SetGrid();
  
    gPad->SetGrid();
    TObject* obj1 = objects1[i];        
    TObject* obj2 = objects2[i];        
    //obj1->Write();
    //obj2->Write();    
    if (obj1->InheritsFrom(TH1F::Class())) {
        TH1F* histogram1 = static_cast<TH1F*>(obj1);
            TH1F* histogram2 = static_cast<TH1F*>(obj2);
	    
	    histogram1->SetLineColor(colors[i]);
	    histogram1->SetFillColor(colors[i]);
	    histogram1->SetMarkerColor(colors[i]);
	    histogram1->SetTitleSize(0.08);
	    histogram1->GetYaxis()->SetLabelSize(0.04);
	    histogram1->GetXaxis()->SetLabelSize(0.04);
	    histogram1->GetYaxis()->SetTitleSize(0.04);
	    histogram1->GetXaxis()->SetTitleSize(0.04);
	    histogram2->SetLineColor(1);
	    //histogram2->SetFillColor(colors[i]);
	    histogram2->SetMarkerColor(1);
	    //   histogram2->SetFillColor(colors[i]);
	    //histogram1->SetFillStyle(3001);
	    histogram1->Draw("e2L");
	    histogram2->Draw("L same");
	    gPad->Update();

   //file 	    histogram2->Write();
   //file 	    histogram1->Write();
		
	    TLegend *legend = new TLegend(0.1,0.7,0.48,0.9);
	    legend->AddEntry(histogram1,"Early baseline","f");
	    legend->AddEntry(histogram2,"Late baseline","l");
	    legend->SetFillStyle(0);


	    legend->Draw();
	    gPad->Update();
	
      
      
	    //      cout<<" okay i am her \n";

    }
   
    else {
      cout << "Error: Unsupported object type at index " << i << "!" << endl;
      continue;
    }
   
   // gPad->Update();
  }
//    gPad->Update();
   c->SaveAs(name+"_c.C");
   myfile->Close();
  TString  new_name(name);
  new_name =  new_name.ReplaceAll(".DXD","");
  c->SaveAs(new_name+".pdf");
  c->SaveAs(new_name+".png");
//    c->SaveAs(name+".png");
     //file   c->Write("c"+name);
    //delete c;


}


void CombineMeanAndRMS(Double_t& combinedMean, Double_t& combinedRMS, const std::vector<Double_t>& meanValues,const std::vector<Double_t>& rmsValues, int numMeasurements)
{
    Double_t sumSquares = 0.0;
 //   combinedMean=0;
 
    if (meanValues.size()<numMeasurements  || rmsValues.size()<numMeasurements) return; 
   // cout<<" num="<<numMeasurements<<"  "<<meanValues.size()<<" "<<rmsValues.size()<<endl;
    for (int i = 0; i < numMeasurements; i++) {
        combinedMean += meanValues[i];
        sumSquares += rmsValues[i] * rmsValues[i];
    }

    combinedMean /= numMeasurements;
    combinedRMS = std::sqrt(sumSquares / numMeasurements);
}


// EOF

