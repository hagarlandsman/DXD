
# Generating the DX2 files:
###############################
# To generate the DX2 processed file, from the binary file, where:
#  calibration/ - Directory with calibration files (depend on the digi speed)
#  DxSetup.txt - Setup file with information on the channels, and processing requested
#  out_0001.dat - This is the file from the digitizer. 
#  out_0001 - Prefix for the output file.   

../src/DxDataExtractor calibration/ DxSetup.txt out_0001.dat out_0001

# Processing the DX2 files:
###########################
# You can look at the wf using the viewer:
python ../mytools/event_viewer_gui.py  out_0001.V3.DX2

# The following script has an example of file processing:
python ../mytools/DX2FileReader.py

