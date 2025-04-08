# 📦 Direxeno Waveform Data Tools

This repository includes:

- ✅ A **C++ binary format writer** for waveform data (with event and channel metadata)
- ✅ A **Python DX2FileReader** for reading and converting the binary data
- ✅ A **Tkinter-based viewer** with waveform browsing, saving, and statistics features
- ✅ A **Python dataExtractor** to easily access events and waveforms
- ✅ Many examples to get you started

---

## 🛠️ C++ Code: Binary File Writer

### File: `src/*.cpp, *.c, *.h`

This code takes digitized waveform data and writes it to a custom `.DX2` binary format. The format includes:

- Per-event headers with event number, timestamp, etc.
- Per-channel blocks with waveform arrays and metadata

### Compiling:
```bash
g++ -o DxDataExtractor DigDataExt.cpp -std=c++11
```

### Running:
```bash
./DxDataExtractor calibration//DxSetup.txt input_file.dat output_prefix
```

Output: `output_prefix.DX2` binary file, and optionally `.DXM` metadata.

### DX2 format:



Format Version 2 - from 8/4/2025
| Step | Field               | Data Type | Size (bytes)                  | Details                                                                                                                                                                                                                     |
|------|---------------------|-----------|-------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1    | EVT_STA             | Tag       | Variable                      | A tag labeled `"EVT_STA"` is written to signify the beginning of an event.                                                                                                                                                  |
| 1.1  | FORMAT_VERSION      | int       | 4                             | ID number of the format  |
|      | **For each channel within the event, repeat the following:** |           |                               |                                                                                                                                                                                                                             |
| 2    | CH__STA             | Tag       | Variable                      | A tag labeled `"CH__STA"` is written to denote the start of channel-specific data.
| 2.3  | channel_data_size  | int       | 4                             |size of the rest of the this channel's data .
| 2.4  | e                   | int       | 4                             | Event number, linking the channel data to its corresponding event.                                                                                                                                                          |
| 2.5  | TimeTag             | long      | 8                             | Timestamp indicating when the data was recorded.                                                                                                                                                                            |
| 2.6  | Tsamp               | float     | 4                             | Sampling period, representing the time interval between consecutive samples in the waveform.                                                                                                                                |
| 2.7  | StartIndex          | float     | 4                             | Starting index of the data within the waveform.                                                                                                                                                                             |
| 2.8  | g                   | int       | 4                             | Group number, classifying the channel into a specific group.                                                                                                                                                                |
| 2.9  | c                   | int       | 4                             | Physical channel number, representing the physical identifier of the channel.                                                                                                                                               |
| 2.10  | ch                  | int       | 4                             | Logical channel number, providing a logical identifier for the channel.                                                                                                                                                     |
| 2.11  | name                | char[]    | FIXED_STRING_LEN=32  | Channel name written as a fixed-length string. A buffer of size `FIXED_STRING_LEN` is created, initialized to zero, and the channel name is copied into this buffer to ensure consistent size in the binary file.            |
| 2.12  | PMTChMap[ch]        | int       | 4                             | PMT (Photomultiplier Tube) map value corresponding to the channel, providing mapping information related to the PMT configuration.                                                                                           |
| 2.13 | waveform            | float[]   | TimeSamples * sizeof(float)   | Array containing the recorded waveform samples. The number of samples (`TimeSamples`) determines the length of this array, representing the actual recorded data for the channel.                                            |


version 1 - used 7-8/4/2025 - obsolete

| Step | Field               | Data Type | Size (bytes)                  | Details                                                                                                                                                                                                                     |
|------|---------------------|-----------|-------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1    | EVT_STA             | Tag       | Variable                      | A tag labeled `"EVT_STA"` is written to signify the beginning of an event.                                                                                                                                                  |
| 1.1  | FORMAT_VERSION      | int       | 4                             | ID number of the format  |
| 1.2  | channel_data_size   | int       | 4                             | Total size of the channel data for this event.                                                                                                                                                                              |
|      | **For each channel within the event, repeat the following:** |           |                               |                                                                                                                                                                                                                             |
| 2    | CH__STA             | Tag       | Variable                      | A tag labeled `"CH__STA"` is written to denote the start of channel-specific data.
| 2.1  | FORMAT_VERSION      | int       | 4                             | ID number of the format used.
| 2.2    | CH__STA             | Tag       | Variable                      | A tag labeled `"CH__STA"` is written to denote the start of channel-specific data.
| 2.3  | channel_data_size  | int       | 4                             |size of the rest of the this channel's data .
| 2.4  | e                   | int       | 4                             | Event number, linking the channel data to its corresponding event.                                                                                                                                                          |
| 2.5  | TimeTag             | long      | 8                             | Timestamp indicating when the data was recorded.                                                                                                                                                                            |
| 2.6  | Tsamp               | float     | 4                             | Sampling period, representing the time interval between consecutive samples in the waveform.                                                                                                                                |
| 2.7  | StartIndex          | float     | 4                             | Starting index of the data within the waveform.                                                                                                                                                                             |
| 2.8  | g                   | int       | 4                             | Group number, classifying the channel into a specific group.                                                                                                                                                                |
| 2.9  | c                   | int       | 4                             | Physical channel number, representing the physical identifier of the channel.                                                                                                                                               |
| 2.10  | ch                  | int       | 4                             | Logical channel number, providing a logical identifier for the channel.                                                                                                                                                     |
| 2.11  | name                | char[]    | FIXED_STRING_LEN=32  | Channel name written as a fixed-length string. A buffer of size `FIXED_STRING_LEN` is created, initialized to zero, and the channel name is copied into this buffer to ensure consistent size in the binary file.            |
| 2.12  | PMTChMap[ch]        | int       | 4                             | PMT (Photomultiplier Tube) map value corresponding to the channel, providing mapping information related to the PMT configuration.                                                                                           |
| 2.13 | waveform            | float[]   | TimeSamples * sizeof(float)   | Array containing the recorded waveform samples. The number of samples (`TimeSamples`) determines the length of this array, representing the actual recorded data for the channel.                                            |


| |what    | size | details |
| 1|EVT_STA |    | A tag labeled "EVT_STA" is written to signify the beginning of an event. |
| 1.1|FORMAT_VERSION | int | ID number of format used |
| For each channel this repeats: | | |
| 1.2 |
The format version (FORMAT_VERSION) is written as an integer.

For Each Channel within the Event:

Channel Start Tag:

A tag labeled "CH__STA" is written to denote the start of channel-specific data.




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
- Metadata support
- Provides: `df_events`, `df_channels`

### Usage:
```python
from DX2FileReader import EventFileReader
reader = EventFileReader("output_prefix.DX2")
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


