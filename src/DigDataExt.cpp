// ==============================================================================
// Digitizer Data Extractor Class Implementation
// ==============================================================================
// Peter Szabo
// Created 02/09/2018
// Updated 31/12/2018
// Modified by Hagar on May 2023 - to include time stamps, and to generate monitoring info
// Modified by Hagar on Apr 2025 - a new file format including more data.
// Add per channel : power by time

// Check titles on all plots
// make a complete run

#include "algorithm"
#include <cstdlib>
#include "TMath.h"
#include "DigDataExt.h"
#include <iomanip>
#include <ctime>

#include <cmath>
#include <string>
#include "DirexenoCorrection.h"
#include <cstring> // for strtok
// #include <iomanip> // for std::quoted
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

const int FORMAT_VERSION = 1;  // or 1001, or 0x01 depending on your scheme

// Fix gera's plot
// Check and add to file saves

// Change back to have h2 with voltage and not power
// subtract baseline from "Gera's plot"
// make a baseline subtracted version of other 2d plot
time_t parse_datetime_to_epoch(const std::string& datetime) {
  std::tm tm = {};
  std::istringstream ss(datetime);
  ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (ss.fail()) {
      std::cerr << "Failed to parse datetime: " << datetime << "\n";
      return -1;
  }
  tm.tm_isdst = -1;  // Let the system determine DST
  return std::mktime(&tm);
}


std::vector<int> parse_int_list(const std::string& s) {
  std::vector<int> result;
  std::string trimmed = s.substr(1, s.size() - 2); // Remove [ ]
  std::stringstream ss(trimmed);
  std::string num;
  while (std::getline(ss, num, ',')) {
      result.push_back(std::stoi(num));
  }
  return result;
}

std::vector<std::string> parse_string_list(const std::string& s) {
  std::vector<std::string> result;
  size_t start = s.find('[');
  size_t end = s.find(']');
  if (end > s.length()) end = s.length();
  std:cout<<s<<" start="<<end<<endl;

  if (start == std::string::npos || end == std::string::npos || end <= start) return result;
  std::string content = s.substr(start + 1, end - start - 1);
  std::stringstream ss(content);
  std::string item;
  std::cout<<"Parsing "<<start<<" "<<end<<"  ss="<<endl;
  while (std::getline(ss, item, ',')) {
      // Trim whitespace
      item.erase(0, item.find_first_not_of(" \t\r\n"));
      item.erase(item.find_last_not_of(" \t\r\n") + 1);
      result.push_back(item);
  }

  return result;
}



std::vector<std::string> splitString(const std::string &input, char delimiter)
{
  std::vector<std::string> substrings;
  std::istringstream iss(input);
  std::string token;

  while (std::getline(iss, token, delimiter))
  {
    substrings.push_back(token);
  }

  return substrings;
}

// std::string base = base_name<std::string>("some/path/file.ext");


void DigDataExt::write_tag(std::ostream& fout, const char* tag) {
  char buffer[8] = {0};
  std::strncpy(buffer, tag, 7);  // max 7 chars + null
  fout.write(buffer, 8);
}


// Read and parse configuration file, set class parameters
int DigDataExt::ParseSetupFile(const std::string SetupFileName)
{
  std::ifstream file(SetupFileName);
  std::string line;
  if (!file)
  {
    cerr << "*** File opening error. ***" << endl
         << endl;
    exit(1);
  }
  while (std::getline(file, line)) {
    // Trim leading/trailing whitespace
    line.erase(0, line.find_first_not_of(" \t\r\n"));
    line.erase(line.find_last_not_of(" \t\r\n") + 1);

    // Skip comments or empty lines
    if (line.empty() || line[0] == '#') continue;

    // Remove inline comments
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
        line = line.substr(0, comment_pos);
    }
    std::stringstream ss(line);
    std::string key, value;
   // ss >> key >> value;
    ss >> key;
    std::getline(ss >> std::ws, value);


    if (key == "PMT_NUM") { //
        PMTNum = std::stoi(value);
        std::cout << "PMT_NUM = " << PMTNum<<"\n";
    }
    else if (key == "PMT_CH_MAP") {
      std::vector<int> temp = parse_int_list(value);

      if (temp.size() > 36) {
        std::cerr << "Too many elements for PMT_NAME_MAP!\n";
        // handle error
    } else {
        std::copy(temp.begin(), temp.end(), PMTChMap);
    }



        std::cout << "PMT_CH_MAP = [ ";
        for (int v : PMTChMap) std::cout << v << " ";
        std::cout << "]\n";
    }



    else if (key == "PMT_NAME_MAP") {
       std::cout<<" XX PMT_NAME_MAP="<<value<<endl;
      PMTNameMap = parse_string_list(value);

      if (PMTNameMap.size() > 36) {
        std::cerr << "Too many elements for PMT_NAME_MAP!\n";
    } else {
        std::cout << "PMT_NAME_MAP = [ ";
        for (const auto& s : PMTNameMap) std::cout << "\"" << s << "\" ";
        std::cout << "]\n";

        std::cout << "First name: " << PMTNameMap[1]
                  << " | Size: " << PMTNameMap.size() << std::endl;
    }
}
    else if (key == "TIME_SAMPLES") { //
      TimeSamples = std::stoi(value);
        std::cout << "TIME_SAMPLES = " << TimeSamples << "\n";
    }
    else if (key == "TIME_RES") {  //
        TimeRes = std::stod(value);
        std::cout << "TIME_RES = " << TimeRes << "\n";
    }
    else if (key == "SIG_RES") { //
        SigRes = std::stoi(value);
        std::cout << "SIG_RES = " << SigRes << "\n";
    }
    else if (key == "CORR_MASK") {//
      value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
      CorrMask = (int)bitset<3>(value).to_ulong();
      std::cout << "CORR_MASK = \t0d" << CorrMask << " = 0b" << bitset<3>(CorrMask) << "   (bits: |time|sample|cell|)" << endl;
    }
    else if (key == "CORR_PATH") {
       // CORR_PATH = value;
        std::cout << "Not using CORR_PATH = \n";// << CORR_PATH << "\n";
    }
    else if (key == "RUN_START") {

        StartTime=parse_datetime_to_epoch(value);
        std::cout << "RUN_START = " << value<<"  "<<std::asctime(std::localtime(&StartTime)) << " epoch="<<StartTime<<"\n";

    }
    else if (key == "RUN_END") {
        EndTime=parse_datetime_to_epoch(value);
        std::cout << "RUN_END = " << value << " epoch="<<EndTime<<"\n";

    }
  }




 /*
  string end_time("");
  string start_time("");
  string end_date("");
  string start_date("");

  bitset<10> param_set("0000000000");
*/


  std::cout << "vvv"<<(int)PMTNameMap.size()<<endl;

  for (int i = 0; i < (int)PMTNameMap.size(); i++)
  {
    string n="";
    if ((int)PMTNameMap.size()>i) n=PMTNameMap[i];
    int ch=-999;
    //if (chn_cnt>i)
    ch=PMTChMap[i];
    std::cout << "\t | "<<i<<"\t | "<<ch<<"\t | "<<n<<"\t |\n";
  }
  return 0;
}

// Load raw data, correct and serialize it, and write it to output file

// DigDataExt::DigDataExt(const string CorrFilesPath, const string setup_file_name, const string dig_file_name, const string out_file_name): SetupFileName(setup_file_name), DigFileName(dig_file_name), OutFileName(out_file_name)


DigDataExt::DigDataExt(const string CorrFilesPath, const string setup_file_name, const string dig_file_name, const string out_file_name, int fast) : SetupFileName(setup_file_name), DigFileName(dig_file_name), OutFileName(out_file_name)
{
  string short_name = out_file_name.substr(out_file_name.find_last_of("/\\") + 1, out_file_name.find_last_of("."));
  std::cout << "----" << short_name << "----" << endl;
  TString root_file_name(OutFileName);
  root_file_name = root_file_name + ".root";
  // load setup parameters
  for (int i = 0; i < 30; i++)
    PMTTitle.push_back(Form("Channel %d", i));
 // cout << PMTTitle.size() << endl;
  cout << "Starting DigDataExt class." << endl;
  ParseSetupFile(SetupFileName);
  // load correction tables from files
  // if (CorrFilesPath.length() > 0 && CorrFilesPath.back() != '/')
  //  {
  //     CorrFilesPath += '/';
  //    }
  cout << "...Loading correction tables from:" << CorrFilesPath << endl;
  CAEN_DGTZ_DRS4Correction_t *CorrTable[MAX_X742_GROUP_SIZE];
  for (int i = 0; i < MAX_X742_GROUP_SIZE; i++)
  {
    // CorrTable[i] = (CAEN_DGTZ_DRS4Correction_t*) malloc(sizeof(CAEN_DGTZ_DRS4Correction_t));
    CorrTable[i] = new CAEN_DGTZ_DRS4Correction_t;
    string s = CorrFilesPath + "X742Table_gr" + to_string(i);
   // cout << s << "_*.txt   (* = cell, nsample, time)" << endl;
    if (0 > LoadCorrectionTable((char *)s.c_str(), CorrTable[i]))
    {
      cerr << "*** Correction table " << s << " read error. ***" << endl
           << endl;
      exit(1);
    }
  }
  int cellCorrection =CorrMask & 0x1;
  int nsampleCorrection = (CorrMask & 0x2) >> 1;
  int timeCorrection = (CorrMask & 0x4) >> 2;
  if (cellCorrection) printf("\t...Will apply cell correction.\n");
  else printf("\t...Will skip cell correction.\n");
  if (nsampleCorrection) printf("\t...Will apply nsample correction.\n");
  else printf("\t...Will skip nsample correction.\n");
  if (timeCorrection) printf("\t...Will apply time correction.\n");
  else printf("\t...Will skip time correction.\n");


  float Tsamp; // in nano-sec
  CAEN_DGTZ_DRS4Frequency_t SampFreq;
  if (TimeRes == 5e9)
  {
    SampFreq = CAEN_DGTZ_DRS4_5GHz;
    Tsamp = (float)((1.0 / 5000.0) * 1000.0);
  }
  else if (TimeRes == 2.5e9)
  {
    SampFreq = CAEN_DGTZ_DRS4_2_5GHz;
    Tsamp = (float)((1.0 / 2500.0) * 1000.0);
  }
  else if (TimeRes == 1e9)
  {
    SampFreq = CAEN_DGTZ_DRS4_1GHz;
    Tsamp = (float)((1.0 / 1000.0) * 1000.0);
  }
  else if (TimeRes == 7.5e8)
  {
    SampFreq = CAEN_DGTZ_DRS4_750MHz;
    Tsamp = (float)((1.0 / 750.0) * 1000.0);
  }
  else
  {
    cerr << "*** Unknown sampling frequency. ***" << endl
         << endl;
    exit(1);
  }

  // open digitizer data file
  if (4 != sizeof(float))
  {
    cerr << "*** sizeof(float) is not 4 bytes, but " << sizeof(float) << " bytes. ***" << endl
         << endl;
    exit(1);
  }
  cout << "Opening digitizer raw data file " << DigFileName << " ." << endl;

  fstream DigDataFile(DigFileName, ios::in | ios::binary);
  if (!DigDataFile)
  {
    cerr << "*** File opening error. ***" << endl
         << endl;
    exit(1);
  }
  DigDataFile.seekg(0, DigDataFile.end);
  DataBufferSize = DigDataFile.tellg();
  DigDataFile.seekg(0);
  printf("\t...buffer size = %d bytes, %f MB \n", DataBufferSize, 1. * DataBufferSize / 1024 / 1024);
  // read raw data to memory
  DataBuffer = new char[DataBufferSize];
  if (nullptr == DataBuffer)
  {
    cerr << "*** Memory allocation error (DataBuffer). ***" << endl
         << endl;
    exit(1);
  }
  DigDataFile.read(DataBuffer, DataBufferSize);
  if (!DigDataFile.good())
  {
    cerr << "*** Data read error (DataBuffer). ***" << endl
         << endl;
    exit(1);
  }
  GetNumEvents(DataBuffer, DataBufferSize, (unsigned int *)&NumOfEvents);
  NumOfEvents = NumOfEvents - 1; // dump last event (often corrupt)
  cout << "\t...File has " << int(NumOfEvents) << " events (last event was dumped)." << endl;

  // open output data file
  cout << "...Opening output data file " << OutFileName << " ." << endl;
  fstream OutDataFile(OutFileName + ".DXD", ios::out | ios::app | ios::binary);

  if (!OutDataFile)
  {
    cerr << "*** File opening error. ***" << endl
         << endl;
    exit(1);
  }

  fstream OutDataFileMeta(OutFileName + ".DX2", ios::out | ios::app | ios::binary);

  if (!OutDataFileMeta)
  {
    cerr << "*** File opening error. ***" << endl
         << endl;
    exit(1);
  }





  int Nmax = NumOfEvents; // NumOfEvents/3;// /30;
  long int roundTime = pow(2, 30);
  // long int roundTime_usec=roundTime*8.5/1000;
  CAEN_DGTZ_X742_EVENT_t *Event;
  char *EventPtr;
  //  DirexenoCorrection *dxc = new DirexenoCorrection("./210223_BL.npz", TimeSamples);
  long int preTime = 0;

  long int T_first = -999;
  int tloop = -1;
  // Get the duration of the run....this might take a while:
  printf("...Looping on all events to get run duration....this might take a few secondsa....\n");
/*
  Start of loop on events in binary out file

*/
  if (1 == 1)
  {
    for (int e = 0; e < Nmax; e++)
    {

      write_tag(OutDataFileMeta, "EVT_STA");
      OutDataFileMeta.write((char *)&FORMAT_VERSION, sizeof(int));  // Event number

      EventPtr = NULL;
      GetEventPtr(DataBuffer, DataBufferSize, e, &EventPtr);
      Event = NULL;
      X742_DecodeEvent(EventPtr, (void **)&Event); // <===== memory leak is here
      long int TimeTag = (long int)Event->DataGroup[0].TriggerTimeTag;
      if (preTime != -999)
      {
        if (TimeTag + tloop * roundTime < preTime)
        {
          tloop++;
          //	    cout<<"tloop="<<tloop<<endl;
        }
      }
      if (T_first == -999)
      T_first = TimeTag;
      TimeTag = TimeTag + tloop * roundTime;
      preTime = TimeTag;
    }
  }
  double_t T_last_usec = preTime * 8.5 / 1000;
  double_t T_first_usec = T_first * 8.5 / 1000;

  cout << "\t...By processed events:"
        <<"\tTfirst = " << (T_first_usec)<<" usec"
        << "\tTlast = " << (T_last_usec) << " usec"
        << "\tdt = " << (T_last_usec - T_first_usec) << " usec"
        << "="<<(T_last_usec - T_first_usec)/1000000<<" sec \t = " << tloop << endl;
  ;
  cout<<"\t...According to setup file:"
  <<"\t start time is: "<<std::asctime(std::localtime(&StartTime))
  <<"\t end time is: "<<std::asctime(std::localtime(&EndTime))
  <<"\t...dt="<<(EndTime-StartTime)<<" sec";


  int nbins = 1e-6 * (T_last_usec - T_first_usec) * 5; // 2d histogram bin size is 0.2 sec
  if (nbins < 5)
    nbins = 10;
  if (nbins > 200)
    nbins = 200;

  tloop = -1;
  TString wf_title[5] = {"sample WF A", "sample WF B", "sample WF C", "sample WF D", "sample WF E"};
  printf("Nmax= %d \n", Nmax);
  ///// Main loop begins
  //long int preTime_usec = 0;
  int nch=0;
  int nev=0;
  for (int e = 0; e < Nmax; e++)
  {

    nev++;
    EventPtr = NULL;

    GetEventPtr(DataBuffer, DataBufferSize, e, &EventPtr);
    Event = NULL;
    X742_DecodeEvent(EventPtr, (void **)&Event); // <===== memory leak is here
    // correct data
    for (int j = 0; j < MAX_X742_GROUP_SIZE; j++)
    {
      // cout<<"Before = "<<Event->DataGroup[j].DataChannel[7][634]<<endl;
      ApplyDataCorrection(CorrTable[j], SampFreq, CorrMask, &Event->DataGroup[j]);
      // cout<<"After = "<<Event->DataGroup[j].DataChannel[7][634]<<endl;
    }


    long int TimeTag = (long int)Event->DataGroup[0].TriggerTimeTag;
   // long int TimeTag_usec = (long int)Event->DataGroup[0].TriggerTimeTag * 8.5 / 1000;
    for (int g = 1; g < 4; g++)
      if (TimeTag != (long int)Event->DataGroup[g].TriggerTimeTag)
        cout << "*************     Time tags of groups are different   ******" << endl;

    // if (preTime!=-999) {
    if (TimeTag + tloop * roundTime < preTime)
      tloop++;
    //}
    if (T_first == -999)
      T_first = TimeTag;
    TimeTag = TimeTag + tloop * roundTime;
    //TimeTag_usec = TimeTag * 8.5 / 1000;
   // long int TimeTag0_usec = TimeTag_usec - T_first_usec;

    //}
    // previous_time_tag = TimeTag;

    preTime = TimeTag;
   // preTime_usec = preTime * 8.5 / 1000;

    // ch is the number of pmt.   c,g is the number of the channel on digitizer
    // keep
    int nok = 0;
   // printf("PMT num = %d \n", PMTNum);
    for (int ch = 0; ch < (PMTNum + -0 * MAX_X742_GROUP_SIZE); ch++)
    {
      int g, c;

      if (ch < PMTNum)
      {                                                      // PMT channel
        g = PMTChMap[ch] / (int)(MAX_X742_CHANNEL_SIZE - 1); // MAX_X742_CHANNEL_SIZE = 9 due to the fast trigger channels (see CAENDigitizerType.h)
        c = PMTChMap[ch] % (int)(MAX_X742_CHANNEL_SIZE - 1);
        nok++;
        //	  printf (" ch= %d \t PMT= %d \t g= %d \t c= %d \n",ch,PMTChMap[ch],g,c);
      }
      else
      { // fast trigger channel
        g = ch - PMTNum;
        c = MAX_X742_CHANNEL_SIZE - 1;
      }
      // check data length integrity (all data vectors should be TimeSamples in length)
      if (TimeSamples != Event->DataGroup[g].ChSize[c])
      {
        cerr << "*** Event " << e << " PMT/TR " << ch << " data length is not " << TimeSamples << " ***" << endl
             << endl;
        break;
        exit(1);
      }

      float StartIndex = (float)Event->DataGroup[g].StartIndexCell;


      // Prepare channel name for binary output as fixed length string.
      std::string& str = PMTTitle[ch];
      char buffer[FIXED_STRING_LEN] = {0};
      // Copy up to 31 characters + null-terminator (optional)
      std::strncpy(buffer, str.c_str(), FIXED_STRING_LEN - 1);


         //	cout<<"baseline_i:"<<g_baseline_i_ch[ch]->GetX()[nn]<<"\t"<<g_baseline_i_ch[ch]->GetY()[nn]<<g_baseline_i_ch[ch]->GetErrorY(nn)<<endl;
      // first write PMT channel data to file by PMT order, and then write the four fast-trigger channels
      // save data to file


      // for positive waveform
      // int locmax = TMath::LocMax(gtemp->GetN(),y);
      //  int sign = +1;

      // for negative wafform


      // Add counting of number of events with sigma over entire run per channel


      // Moshe: need to seperate event and channels
      OutDataFile.write((char *)&TimeTag, sizeof(float));
      OutDataFile.write((char *)&StartIndex, sizeof(float));
      OutDataFile.write((char *)Event->DataGroup[g].DataChannel[c], TimeSamples * sizeof(float));



      if (!OutDataFile.good())
      {
        cerr << "*** File writing error. ***" << endl
             << endl;
        exit(1);
      }

      std::vector<float> wf(Event->DataGroup[g].DataChannel[c],
        Event->DataGroup[g].DataChannel[c] + TimeSamples);

      WriteChannelToMeta(OutDataFileMeta, e, TimeTag, Tsamp, StartIndex, g, c, ch,  PMTTitle[ch], PMTChMap[ch], wf);


      if (!OutDataFileMeta.good())
      {
        cerr << "*** Meta File writing error. ***" << endl
             << endl;
        exit(1);
      }




    } // End of Loop on PMTs channels

   // printf("Nch= %d, nev=%d <=============== \n", nch,nev);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // free memory allocated by X742_DecodeEvent
    char chanMask = 15;

    // This is a fix to the memory leak. HYL 20/5/2023
    for (int g = 0; g < X742_MAX_GROUPS; g++)
    {
      if ((chanMask >> g) & 0x1)
      {
        for (int j = 0; j < MAX_X742_CHANNEL_SIZE; j++)
        {
          // Event->DataGroup[g].DataChannel[j]= malloc(X742_FIXED_SIZE * sizeof (float));
          free(Event->DataGroup[g].DataChannel[j]);
        }
      }
    }
    // Fix ended

    delete Event;
    cout << "\r";
    int PrgBarWidth = 50;
    float Prg = (e + 1) / (float)NumOfEvents;
    cout << "[";
    int PrgPos = PrgBarWidth * Prg;
    for (int i = 0; i < PrgBarWidth; ++i)
    {
      if (i < PrgPos)
        cout << "=";
      else if (i == PrgPos)
        cout << ">";
      else
        cout << " ";
    }
    cout << "] " << int(Prg * 100.0) << " % (" << (e + 1) << "/"<<Nmax<<" events)";
    cout.flush();

  } // End of Loop on e Nevents
  cout << "done..."<<endl; // end the progress bar's last line update

  // free correction tables memory
  for (int i = 0; i < MAX_X742_GROUP_SIZE; i++)
  {
    // free(CorrTable[i]);
    delete CorrTable[i];
  }

  // close and finish
  DigDataFile.close();
  OutDataFile.close();
  OutDataFileMeta.close();
  return;

}


void DigDataExt::WriteChannelToMeta(std::fstream& Out,
  int e, long TimeTag, float Tsamp, float StartIndex,
  int g, int c, int ch, const std::string& name,
  int pmt_ch, const std::vector<float>& waveform)
  {
    //printf ("WF - %d %d %f %f \n",e,ch,waveform[0],waveform[1]);
    write_tag(Out, "CH__STA");
    int channel_data_size =
    sizeof(int) +        // event number
    sizeof(long) +       // time tag
    sizeof(float) +      // tsamp
    sizeof(float) +      // start index
    sizeof(int) * 3 +     // g, c, ch
    FIXED_STRING_LEN +   // name string
    sizeof(int) +        // PMT map value
    TimeSamples * sizeof(float);  // waveform
    Out.write(reinterpret_cast<const char*>(&FORMAT_VERSION), sizeof(int));
    write_tag(Out, "CH__STA");
    Out.write((char *)&channel_data_size, sizeof(int));  // Event number
    Out.write((char *)&e, sizeof(int));  // Event number
    Out.write((char *)&TimeTag, sizeof(long));
    Out.write((char *)&Tsamp, sizeof(float));
    Out.write((char *)&StartIndex, sizeof(float));
    Out.write((char *)&g, sizeof(int));  // group number
    Out.write((char *)&c, sizeof(int));  // physical channel number
    Out.write((char *)&ch, sizeof(int));  // logical channel number
    // Write name (fixed-length)
    char name_buf[FIXED_STRING_LEN] = {0};
    std::strncpy(name_buf, name.c_str(), FIXED_STRING_LEN - 1);
    Out.write(name_buf, FIXED_STRING_LEN);
    Out.write((char *)&PMTChMap[ch], sizeof(int)); //
    Out.write((char*)waveform.data(), TimeSamples * sizeof(float));



  }




// EOF
