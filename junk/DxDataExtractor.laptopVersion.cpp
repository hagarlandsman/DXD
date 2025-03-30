// ==============================================================================
//    DxDataExtractor - extract raw V1742 digitizer data to a binary file         
// ==============================================================================
// Peter Szabo 14/10/2018
//
// A tool to extract raw CAEN V1742 digitizer data into a simple sequential
// binary file (DireXeno Data file, or DXD), to be used in the analysis.
//

#include <string>
#include <iostream>
#include <fstream>
#include "DigDataExt.h"
#include <cstdlib>

using namespace std;

int main(int argc, char const *argv[])
{
    cout << "==============================================================================" << endl;
    cout << "   DxDataExtractor - extract raw V1742 digitizer data to a binary file       " << endl;
    cout << "==============================================================================" << endl;
    if (argc == 4)
    {
        // check if output file exists
        fstream OutFile(argv[3]);
        bool exists = OutFile.good();
        OutFile.close();
        if (!exists)
        {
            cout << "Creating a new output file " << argv[3] << " ." << endl;
            OutFile.open(argv[3], ios::out|ios::trunc); // create an empty file
            if (!OutFile) { cerr << "*** File opening error. ***" << endl << endl; exit(1); }
            OutFile.close();
        }
        else
        {
            cout << "Appending existing output file " << argv[3] << " ." << endl;
        }
	cout<<"start \n";
        DigDataExt dex = DigDataExt(argv[1], argv[2], argv[3]);
	cout<<"done \n";
    }
    else
    { 
        cerr << "A tool to extract raw CAEN V1742 digitizer data (DIG file) into a sequential" << endl;
        cerr << "and PMT oriented binary data-stream (DXD file), which is employed in analysis." << endl;
        cerr << "The system and digitizer parameters are given in a setup text-file." << endl;
        cerr << endl;
        cerr << "Usage: DxDataExtractor <SETUP_FILE> <DIG_FILE> <DXD_FILE>" << endl;
        cerr << endl;
        cerr << "       NOTE:   If DXD_FILE does not exist, it is created." << endl;
        cerr << "               If it does exist, it is appended with the" << endl;
        cerr << "               new events." << endl;
        cerr << endl;
        exit(1);
    }

    return 0;
}



// EOF
