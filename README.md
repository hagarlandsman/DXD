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
| 2.6  | g                   | int       | 4                             | Group number, classifying the channel into a specific group.                                                                                                                                                                |
| 2.7  | c                   | int       | 4                             | Physical channel number, representing the physical identifier of the channel.                                                                                                                                               |
| 2.8 | ch                  | int       | 4                             | Logical channel number, providing a logical identifier for the channel.                                                                                                                                                     |
| 2.9  | name                | char[]    | FIXED_STRING_LEN=32  | Channel name written as a fixed-length string. A buffer of size `FIXED_STRING_LEN` is created, initialized to zero, and the channel name is copied into this buffer to ensure consistent size in the binary file.            |
| 2.10  | PMTChMap[ch]        | int       | 4                             | PMT (Photomultiplier Tube) map value corresponding to the channel, providing mapping information related to the PMT configuration.                                                                                           |
| 2.11 | waveform            | float[]   | TimeSamples * sizeof(float)   | Array containing the recorded waveform samples. The number of samples (`TimeSamples`) determines the length of this array, representing the actual recorded data for the channel.                                            |



### Setup File Format:
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

### Usage:
```python
from DX2FileReader import *

reader = DX2("data_file.DX2")
print("Data frame containing the channels mapping information:")
print(reader.df_channels.head())

print("Data frame containing the data information:")
print(reader.df_events.head())

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


