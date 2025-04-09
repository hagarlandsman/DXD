import struct
import pandas as pd
import matplotlib.pyplot as plt
from collections import defaultdict
import numpy as np
import mmap

def search_binary_file(file_path, byte_sequence):
    with open(file_path, 'rb') as f:
        # Memory-map the file, size 0 means whole file
        offset = 0
        with mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as mm:
            offset = mm.find(b'CH__STA\x00',offset)
            if offset == -1:
                    return (-1)
            print(f"Match found at offset:{offset}")
            offset += len(b'CH__STA\x00')  # Move past the last found occurrence
            offset = mm.find(b'EVT_STA\x00',offset)
            print(f"Match found at offset: {offset}")
            offset += len(b'CH__STA\x00')  # Move past the last found occurrence
            offset = mm.find(b'EVT_STA\x00',offset)
            print(f"Match found at offset: {offset}")
            offset += len(b'CH__STA\x00')  # Move past the last found occurrence
            offset = mm.find(byte_sequence, offset)
            print(f"Match found at offset: {offset}")


class DX2:
    TAG_SIZE = 8
    EVT_TAG = b"EVT_STA\x00"
    CH_TAG = b"CH__STA\x00"
    FIXED_STRING_LEN = 32
    FORMAT_VERSION_SIZE = 4
    FORMAT_EVSIZE_SIZE = 4
    FLOAT_SIZE = 4
    TIME_SAMPLES = 1024
    CHANNEL_HEADER_FORMAT = "<iqffiii"
    EXTRA_FORMAT = f"<{FIXED_STRING_LEN}s i"

    def __init__(self, filename):
        self.filename = filename
        self.df_channels = None
        self.df_events = None
        self.version = -1
        self.event_size_total = -1
        self.ch_size = -1
        self.file_verified = 0
        self.file_size = -1
        self.map_event()
        self.Nevents = 1.*self.file_size/self.event_size_total

        print(f"event_size_total={self.event_size_total}")
        print(f"event_size_total={self.ch_size}")
        print(f"file size is {self.file_size}, N={self.Nevents}")
        if (self.version<3):
            print (f"Current dx2 version is {self.version}. This library works with version >=3")

    def get_event_address(self, i):
        return (i-1)*self.event_size_total

    def goto(self,i):
        address = self.get_event_address(i)
        with open(self.filename, "rb") as f:
            f.seek(address,0)
            tag = f.read(self.TAG_SIZE)
            tag_str = tag.decode('utf-8')
            print ("goto tag = ",tag_str)

            if tag != self.EVT_TAG:
                raise ValueError(f"Expected CH__STA, got {tag}")


    def  map_event(self):  # MOSHE map event not working
        with open(self.filename, "rb") as f:
            while (1):
                print ("hi")
                tag = f.read(self.TAG_SIZE)
                if not tag:
                    print ("Problem processing file (1)")
                    break
                if tag == self.EVT_TAG:
                    version_bytes = f.read(self.FORMAT_VERSION_SIZE)
                    self.version = struct.unpack("<i", version_bytes)[0]
                    if (self.version == 3) :
                        event_size_bytes = f.read(4)
                        event_size = struct.unpack("<i", event_size_bytes)[0]
                        self.event_size_total = event_size + self.FORMAT_VERSION_SIZE + 4 + 8;
                    continue
                elif tag == self.CH_TAG:

                    if (self.version==1):   # Only These lines part was in version1, but  removed at version 2, redundant
                        version_bytes = f.read(self.FORMAT_VERSION_SIZE)
                        version = struct.unpack("<i", version_bytes)[0]

                        tag2 = f.read(self.TAG_SIZE)


                        if tag2 != self.CH_TAG:
                            raise ValueError(f"Expected CH__STA, got {tag2}")

                    chsize_bytes = f.read(4)
                    self.ch_size = struct.unpack("<i", chsize_bytes)[0]
                    break
                else:
                    print ("Problem,")
                    return
            f.seek(0, 2)  # Move to end of file
            self.file_size = f.tell()


                #file_object.seek(chsize + FORMAT_OFFSET_2, 1)  # 0 from file start (1 from current position)


    def read_event(self, i):

        event_data = defaultdict(lambda: {"time_tag": None, "tsamp": None, "start_index": None, "waveforms": []})
        count = 0
        nevent = 0
        neventch = 0
        waveforms = []
        event = {}
        with open(self.filename, "rb") as f:
            f.seek(self.get_event_address(i),0)
            once = 0
            while True:
                tag = f.read(self.TAG_SIZE)
                tag_str = tag.decode('utf-8')
                here = 0
                if not tag:
                    print ("oops")
                    return (-1)
                if tag == self.EVT_TAG:
                    if (once>0):
                        break
                    once = once + 1
                    version_bytes = f.read(self.FORMAT_VERSION_SIZE)
                    version = struct.unpack("<i", version_bytes)[0]
                    event_size = 0
                    if (version == 3) :
                        event_size_bytes = f.read(4)
                        event_size = struct.unpack("<i", event_size_bytes)[0]
                    count = count + 1
                elif tag == self.CH_TAG:
                    neventch = neventch + 1
                    count = 0
                    if (version==1):   # Only These lines part was in version1, but  removed at version 2, redundant
                        version_bytes = f.read(self.FORMAT_VERSION_SIZE)
                        version = struct.unpack("<i", version_bytes)[0]

                        tag2 = f.read(self.TAG_SIZE)
                        if tag2 != self.CH_TAG:
                            raise ValueError(f"Expected CH__STA, got {tag2}")


                    size_bytes = f.read(4)
                    channel_data_size = struct.unpack("<i", size_bytes)[0]
                    #print ("size = ",channel_data_size)
                    header_bytes = f.read(struct.calcsize(self.CHANNEL_HEADER_FORMAT))
                    e, time_tag, tsamp, start_index, g, c, ch = struct.unpack(self.CHANNEL_HEADER_FORMAT, header_bytes)

                    extra_bytes = f.read(struct.calcsize(self.EXTRA_FORMAT))
                    name_raw, pmt_ch = struct.unpack(self.EXTRA_FORMAT, extra_bytes)
                    name = name_raw.decode('utf-8').rstrip('\x00')

                    waveform_bytes = f.read(self.TIME_SAMPLES * self.FLOAT_SIZE)
                    waveform = struct.unpack(f"<{self.TIME_SAMPLES}f", waveform_bytes)

                    wf ={'logic_ch':ch,'pmt_ch':pmt_ch,'name':name,'group':g,'group_channel':c,'waveform':list(waveform)}

                    waveforms.append(wf)
                    event={'event':1,'tsamp':tsamp,'time_tag':time_tag,'start_index':start_index,'waveforms':waveforms}

                else:
                    raise ValueError(f"Unknown tag found: {tag}")
        return(event)



        return event
    def read_file_new(self):
        n=1
        events_list = []

        while n<self.Nevents:
            d=self.read_event( n)
            events_list.append(d)
            n=n+1

        self.df_events = pd.DataFrame(events_list)
        print ("keys within Events data frame:\t",self.df_events.keys())
        print ("df_events head:")
        print(self.df_events.head())

    def plot_event_waveforms(self, event_number, separate_subplots=False):
        event_row = self.df_events[self.df_events['event'] == event_number]
        if event_row.empty:
            print(f"Event {event_number} not found.")
            return

        waveforms = event_row.iloc[0]['waveforms']
        tsamp = event_row.iloc[0]['tsamp']
        start_index = event_row.iloc[0]['start_index']

        channel_colors = plt.cm.tab20(np.linspace(0, 1, 32))  # assign a unique color to each channel

        if separate_subplots:
            n = len(waveforms)
            nrows, ncols = 5, 6
            fig, axs = plt.subplots(nrows, ncols, figsize=(18, 12), sharex=True)
            axs = axs.flatten()
            for i, wf in enumerate(waveforms):
                logic_ch = wf['logic_ch']
                y = wf['waveform']
                x = [start_index + i * tsamp for i in range(len(y))]
                color = channel_colors[logic_ch % len(channel_colors)]
                axs[i].plot(x, y, color=color)
                axs[i].set_title(f"Ch {logic_ch}: {info['name']}", fontsize=9)
                axs[i].grid(True)
            for j in range(len(waveforms), nrows * ncols):
                axs[j].axis('off')
            plt.tight_layout()
            plt.show()
        else:
            plt.figure(figsize=(12, 6))
            for wf in waveforms:
                logic_ch = wf['logic_ch']
                y = wf['waveform']
                x = [start_index + i * tsamp for i in range(len(y))]
                color = channel_colors[logic_ch % len(channel_colors)]
                plt.plot(x, y, label=f"ch {logic_ch}", color=color)

            plt.title(f"Waveforms for Event {event_number}")
            plt.xlabel("Time")
            plt.ylabel("Amplitude")
            plt.legend()
            plt.grid(True)
            plt.tight_layout()
            plt.show()

    def plot_event_waveforms_frame(self, event_row, title, separate_subplots=False):


        waveforms = event_row['waveforms']
        tsamp = event_row['tsamp']
        start_index = event_row['start_index']

        channel_colors = plt.cm.tab20(np.linspace(0, 1, 32))  # assign a unique color to each channel

        if separate_subplots:
            n = len(waveforms)
            nrows, ncols = 5, 6
            fig, axs = plt.subplots(nrows, ncols, figsize=(18, 12), sharex=True)
            axs = axs.flatten()
            for i, wf in enumerate(waveforms):
                logic_ch = wf['logic_ch']
                y = wf['waveform']
                x = [start_index + i * tsamp for i in range(len(y))]
                color = channel_colors[logic_ch % len(channel_colors)]

                name = wf['name']
                axs[i].plot(x, y, color=color)
                axs[i].set_title(f"Ch {logic_ch}: {name}", fontsize=9)
                axs[i].grid(True)
            for j in range(len(waveforms), nrows * ncols):
                axs[j].axis('off')
            plt.tight_layout()
            plt.show()
        else:
            plt.figure(figsize=(12, 6))
            for wf in waveforms:
                logic_ch = wf['logic_ch']
                y = wf['waveform']
                x = [start_index + i * tsamp for i in range(len(y))]
                color = channel_colors[logic_ch % len(channel_colors)]
                plt.plot(x, y, label=f"ch {logic_ch}", color=color)

            plt.title(title)
            plt.xlabel("Time")
            plt.ylabel("Amplitude")
            plt.legend()
            plt.grid(True)
            plt.tight_layout()
            plt.show()

    def draw_summary(self):
        max_values = defaultdict(list)
        min_values = defaultdict(list)
        peak_bins = defaultdict(list)

        for _, row in self.df_events.iterrows():
            for wf in row['waveforms']:
                ch = wf['logic_ch']
                data = np.array(wf['waveform'])
                max_values[ch].append(np.max(data))
                min_values[ch].append(np.min(data))
                peak_bins[ch].append(np.argmax(data))

        fig, axs = plt.subplots(3, 1, figsize=(12, 12))

        axs[0].set_title("Maximum Values per Channel")
        for ch, vals in max_values.items():
            axs[0].hist(vals, bins=50, alpha=0.6, label=f"ch {ch}")
        axs[0].legend()
        axs[0].set_ylabel("Frequency")

        axs[1].set_title("Minimum Values per Channel")
        for ch, vals in min_values.items():
            axs[1].hist(vals, bins=50, alpha=0.6, label=f"ch {ch}")
        axs[1].legend()
        axs[1].set_ylabel("Frequency")

        axs[2].set_title("Peak Bin Index per Channel")
        for ch, bins_ in peak_bins.items():
            axs[2].hist(bins_, bins=self.TIME_SAMPLES//32, alpha=0.6, label=f"ch {ch}")
        axs[2].legend()
        axs[2].set_xlabel("Sample Bin")
        axs[2].set_ylabel("Frequency")

        plt.tight_layout()
        plt.show()

    def get_channel_info(self, logic_channel_number):
        """Return metadata for a given logic channel number."""
        row = self.df_channels[self.df_channels['logic_ch'] == logic_channel_number]
        if row.empty:
            print(f"No information found for logic channel {logic_channel_number}")
            return None
        return row.iloc[0].to_dict()


# === Example usage ===
if __name__ == "__main__":
    reader = DX2("small.DX2")
   # reader.goto(50)
    d  = reader.read_event(2)
    #reader.read_file_new()
    #reader.plot_event_waveforms(0, separate_subplots=False)

    #print (d )
    reader.plot_event_waveforms_frame(d,1,'hi')
   # print("Channels:")
   # print(reader.df_channels.head())
   # print("\nEvents:")
   # print(reader.df_events.head())
 #   reader.plot_event_waveforms(0, separate_subplots=False)
 #   reader.draw_summary()
