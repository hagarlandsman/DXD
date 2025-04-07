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


