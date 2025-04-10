# 📦 Direxeno Waveform Data Tools

A collection of tools for handling waveform data from V1742 digitizers:

- ✅ `src/`: **C++ binary format writer** — produces the legacy `.DXD` and the modern `.DX2` binary formats  
- ✅ `mytools/`: **Python DX2FileReader** — loads `.DX2` binary files into a structured `pandas` DataFrame  
- ✅ `mytools/`: **Tkinter-based waveform viewer** — interactive browser with plot/export functionality  
- ✅ `examples/`: Example `.DX2` files and usage scripts  

---


## 🛠️ C++ Code: DX2 Binary Writer

### Location: `src/*.cpp, *.c, *.h`

This code takes digitized waveform data and writes it to a custom `.DX2` binary format. Also supports writing the obsolete `.DXD` format (unchanged structure).

The online code reading the data from the digitizer and producing the "out_00001.DAT" files can be found here: https://github.com/hagarlandsman/direxdaq_code/tree/main/xedaq 

### 🔧 Compile:

```bash
make DxDataExtractor
```

### ▶️ Run:

```bash
./DxDataExtractor calibration/DxSetup.txt input_file.dat output_prefix
```

Output:
- `output_prefix.DX2` — main binary file
- `output_prefix.DXD` — legacy file format

---

## 📚 DX2 File Format Overview (Version 3 – from 9 April 2025)

### 🔹 Event Header

| Field            | Type     | Size    | Description                           |
|------------------|----------|---------|---------------------------------------|
| `"EVT_STA"`      | Tag      | 8 bytes | Marks the start of an event           |
| `FORMAT_VERSION` | `int`    | 4       | Format version ID                     |
| `TOTAL_SIZE`     | `int`    | 4       | Size of all remaining event data      |

### 🔸 Per Channel Block (repeats per channel)

| Field              | Type       | Size              | Description                                 |
|--------------------|------------|-------------------|---------------------------------------------|
| `"CH__STA"`        | Tag        | 8 bytes           | Marks start of channel-specific block       |
| `channel_data_size`| `int`      | 4                 | Size of this channel block (not including tag) |
| `e`                | `int`      | 4                 | Event number                                |
| `TimeTag`          | `long`     | 8                 | Timestamp                                   |
| `Tsamp`            | `float`    | 4                 | Sampling period                             |
| `StartIndex`       | `float`    | 4                 | Starting index of the waveform              |
| `g`                | `int`      | 4                 | Digitizer group                             |
| `c`                | `int`      | 4                 | Channel within group                        |
| `ch`               | `int`      | 4                 | Logical channel number                      |
| `name`             | `char[]`   | 32                | Channel name (fixed-size buffer)            |
| `PMTChMap[ch]`     | `int`      | 4                 | Original digitizer channel mapping          |
| `waveform`         | `float[]`  | `TimeSamples * 4` | Array of waveform samples                   |

---

## 🐍 Python: `DX2FileReader`

### Location: `mytools/DX2FileReader.py`

Parses `.DX2` binary files and returns a structured list of events. Each event is represented as a dictionary with metadata and waveform data.

### ✅ Output Structure

Each call to `read_event(n)` returns a dictionary like this:

```python
{
    "event": 1,                        # Event index
    "time_tag": 109650,               # Event time tag (from digitizer)
    "tsamp": 0.2,                     # Sampling period in ns
    "start_index": 958.0,            # Index (float) of trigger within buffer
    "waveforms": [                   # List of waveform dictionaries, one per channel
        {
            "logic_ch": 0,            # Logical channel number
            "pmt_ch": 1,              # Original digitizer channel
            "name": "PMT12",          # Channel name
            "group": 0,               # Digitizer group
            "group_channel": 1,       # Channel within group
            "waveform": [3486.0, 3485.99, 3485.8, ...]  # List of sampled waveform values
        },
        {
            "logic_ch": 1,
            "pmt_ch": 2,
            "name": "PMT5",
            "group": 0,
            "group_channel": 2,
            "waveform": [3479.0, 3479.1, 3479.0, ...]
        },
        ...
    ]
}
```

---

### 🧪 Example usage

```python
import DX2FileReader as DX2

data = DX2.DX2("example/small.DX2")

# Read a single event (event 1)
ev = data.read_event(1)

# Inspect the structure
print(ev.keys())
# dict_keys(['event', 'tsamp', 'time_tag', 'start_index', 'waveforms'])

print(ev['waveforms'][0].keys())
# dict_keys(['logic_ch', 'pmt_ch', 'name', 'group', 'group_channel', 'waveform'])

print(ev['waveforms'][0]['waveform'][:10])  # Show first 10 samples

# If some reason you want to load the entire file to memory:
data.read_file_new() 
# all events are in df_events data frame:
keys within Events data frame:   Index(['event', 'tsamp', 'time_tag', 'start_index', 'waveforms'], dtype='object')
df_events head:
   event  ...                                          waveforms
0      1  ...  [{'logic_ch': 0, 'pmt_ch': 1, 'name': 'Trigger...
1      1  ...  [{'logic_ch': 0, 'pmt_ch': 1, 'name': 'Trigger...
2      1  ...  [{'logic_ch': 0, 'pmt_ch': 1, 'name': 'Trigger...
3      1  ...  [{'logic_ch': 0, 'pmt_ch': 1, 'name': 'Trigger...
4      1  ...  [{'logic_ch': 0, 'pmt_ch': 1, 'name': 'Trigger...



```



---

### 📊 Plotting events

```python
# Plot a single event with all channels overlaid
DX2.plot_event_waveforms_frame(ev, title="Event 1", overlay=True)

# Plot a single event as multiple subplots (5x6 grid)
DX2.plot_event_waveforms_frame(ev, title="Event 1", overlay=False)

# Or directly using the reader:
data.plot_event_waveforms(2, overlay=True)
```

---

## 🖼️ GUI Viewer

### File: `mytools/event_viewer_gui.py`

Tkinter-based waveform viewer for `.DX2` files.

### Features:

- Browse events using "Next"/"Prev" buttons
- Toggle overlay vs. subplot grid view (5×6)
- Export current event to PNG or CSV
- Display PMT mapping and waveform metadata

### Run it:

```bash
python event_viewer_gui.py output_prefix.DX2
```

Or use the helper wrapper that also extracts data:

```bash
python run_and_view.py calibration DxSetup.txt input_file.dat
```

---



## 📁 File Structure

```
/DXD
├── src/                # C++ waveform extractor
├── mytools/            # Python DX2 reader + GUI viewer
├── examples/           # Sample data and output files
│   ├── input_file.dat
│   ├── DxSetup.txt
│   └── output_prefix.DX2
```
