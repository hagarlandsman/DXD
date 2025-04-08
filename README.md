# 📦 Direxeno Waveform Data Tools

This repository includes:
- ✅ /src:  A **C++ binary format reader** for waveform data V1742 data. Produces the obsolete DXD file, and the new DX2 file.
- ✅ /mytools **Python DX2FileReader** for reading and converting the DX2 binary data into a python numpy dataframe
- ✅ /mytools **Tkinter-based viewer** with waveform browsing, saving, and statistics features
- ✅ /examples  **Python DX2FileReader** for reading and converting the binary data
---

## 🛠️ C++ Code: Binary File Writer

### File: `src/*.cpp, *.c, *.h`

This code takes digitized waveform data and writes it to a custom `.DX2` binary format. 
### Compiling:
```bash
make DxDataExtractor
```

### Running:
```bash
./DxDataExtractor calibration//DxSetup.txt input_file.dat output_prefix
```

Output: `output_prefix.DX2` binary file, and the obsolete `output_prefix.DXD`. DXD format was not changed. 

### DX2 format:

Format Version 2 - from 8/4/2025

  **Event header:**

| Step | Field               | Data Type | Size (bytes)                  | Details                                                                                                                                                                                                                     |
|------|---------------------|-----------|-------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1    | EVT_STA             | Tag       | Variable                      | A tag labeled `"EVT_STA"` is written to signify the beginning of an event.                                                                                                                                                  |
| 1.1  | FORMAT_VERSION      | int       | 4                             | ID number of the format  |
|                                    |                                                                                                                                                                                                                

**For each channel within the event, repeat the following:** 

| Step | Field               | Data Type | Size (bytes)                  | Details                                                                                                                                                                                                                     |
|------|---------------------|-----------|-------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 2    | CH__STA             | Tag       | Variable                      | A tag labeled `"CH__STA"` is written to denote the start of channel-specific data.
| 2.1  | channel_data_size  | int       | 4                             |size of the rest of the this channel's data .
| 2.2  | e                   | int       | 4                             | Event number, linking the channel data to its corresponding event.                                                                                                                                                          |
| 2.3  | TimeTag             | long      | 8                             | Timestamp indicating when the data was recorded.                                                                                                                                                                            |
| 2.4  | Tsamp               | float     | 4                             | Sampling period, representing the time interval between consecutive samples in the waveform.                                                                                                                                |
| 2.5  | StartIndex          | float     | 4                             | Starting index of the data within the waveform.                                                                                                                                                                             |
| 2.6  | g                   | int       | 4                             | Group number,  g = PMTChMap[ch]/8; |                                                                                                                                                                |
| 2.7  | c                   | int       | 4                             |  Channel number withing a group, c= PMTChMap[ch]%8; |                                                                                                                                              |
| 2.8 | ch                  | int       | 4                             | Logical channel number, providing a logical identifier for the channel.                                                                                                                                                     |
| 2.9  | name                | char[]    | FIXED_STRING_LEN=32  | Channel name written as a fixed-length string. A buffer of size `FIXED_STRING_LEN` is created, initialized to zero, and the channel name is copied into this buffer to ensure consistent size in the binary file.            |
| 2.10  | PMTChMap[ch]        | int       | 4                             | Digitizer channel . This is used to calculate "g" and "c"|                                                                                         |
| 2.11 | waveform            | float[]   | TimeSamples * sizeof(float)   | Array containing the recorded waveform samples. The number of samples (`TimeSamples`) determines the length of this array, representing the actual recorded data for the channel.                                            |



### DxSetup.txt Setup File Format:
Unchanged.
- Space-separated `key value` lines
- Arrays: `[1,2,3]`
- Strings: `"Quoted"` or bare words
- Lines starting with `#` are ignored

---

## 🐍 Python: `DX2FileReader`

### File: `DX2FileReader.py`

Use this to parse `.DX2` waveform files into a structured format with Pandas.

### Features:
- Load all events and channel waveforms
- Provides: `df_events`, `df_channels`

keys within df_channels:	 
- 'pmt_ch'
- 'name'
- 'logic_ch'
- 'phys_ch'
- 'group'

```python
Channels:
========
keys within Channels data frame:	 Index(['pmt_ch', 'name', 'logic_ch', 'phys_ch', 'group'], dtype='object')
df_channels head:
   pmt_ch       name  logic_ch  phys_ch  group
0       1  Channel 0         0        1      0
1       3  Channel 1         1        3      0
2       4  Channel 2         2        4      0
3       5  Channel 3         3        5      0
4       6  Channel 4         4        6      0
```


keys within df_events:
- 'event'
- 'time_tag'
- 'tsamp'
-  'start_index',
-   'waveforms'

  ```python

Events:
========
keys within Events data frame:	 Index(['event', 'time_tag', 'tsamp', 'start_index', 'waveforms'], dtype='object')
df_events head:
   event  time_tag  tsamp  start_index                                          waveforms
0      0     62774    0.2        790.0  [{'logic_ch': 0, 'waveform': [3477.0, 3477.0, ...
1      1    109650    0.2        958.0  [{'logic_ch': 0, 'waveform': [3486.0, 3485.994...
2      2    156524    0.2         19.0  [{'logic_ch': 0, 'waveform': [3480.0, 3480.0, ...
3      3    203398    0.2        106.0  [{'logic_ch': 0, 'waveform': [3481.0, 3481.0, ...
4      4    250272    0.2        191.0  [{'logic_ch': 0, 'waveform': [3502.0, 3502.0, ...

```

### Usage:
```python
from DX2FileReader import *

reader = DX2("/home/hagar/Workspace/codes/DXD/example/out_0001.DX2")

print("Channels:\n========")
df_channels = reader.df_channels
print ("keys within Channels data frame:\t",df_channels.keys())
print ("df_channels head:")
print(df_channels.head())

print("\nEvents:\n========")
df_events = reader.df_events
print ("keys within Events data frame:\t",df_events.keys())
print ("df_events head:")
print(df_events.head())

# How to access events:
##############################################

# extract event 10:
print ("Accessing event number 10:")
event_row = df_events[df_events['event'] == 10]

# access the event's WF list:
if not event_row.empty:
    waveforms_list = event_row.iloc[0]['waveforms']

    # Iterate to find channel 4
    for waveform_dict in waveforms_list:
        if waveform_dict['logic_ch'] == 4:
            wf_channel_4 = waveform_dict['waveform']
            #print("Waveform for channel 4 in event 10:", wf_channel_4)
            break
    else:
        print("Channel 4 not found in event 10.")
else:
    print("Event 10 not found.")

# Plot an event using DX2 library:
reader.plot_event_waveforms(0, separate_subplots=False)
reader.draw_summary()

```

---

## 🖼️ Python GUI Viewer

### File: `event_viewer_gui.py`

Tkinter-based interactive viewer for DX2 waveform data.

### Features:
- Forward/backward event navigation
- Toggle between overlay and 5×6 channel subplots
- CSV + PNG export
- Summary plot and waveform comparison tools
- Scrollable PMT info table and colored waveforms

### Run it:
```bash
python event_viewer_gui.py output_prefix.DX2
```

Or use the wrapper to extract & view in one step:
```bash
python run_and_view.py calibration DxSetup.txt input_file.dat
```

---

## 🧪 Requirements
```bash
pip install numpy pandas matplotlib
```
CAEN libraries

Optional:
- `tkinter` (comes with most Python installs)

---

## 📁 File Structure
```
/DXD
├── src
├── mytools
├── examples
│   ├── input_file.dat
│   └── output_prefix.DX2
```


