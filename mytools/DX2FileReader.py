import struct
import pandas as pd
import matplotlib.pyplot as plt
from collections import defaultdict
import numpy as np

class DX2:
    TAG_SIZE = 8
    EVT_TAG = b"EVT_STA\x00"
    CH_TAG = b"CH__STA\x00"
    FIXED_STRING_LEN = 32
    FORMAT_VERSION_SIZE = 4
    FLOAT_SIZE = 4
    TIME_SAMPLES = 1024
    CHANNEL_HEADER_FORMAT = "<iqffiii"
    EXTRA_FORMAT = f"<{FIXED_STRING_LEN}s i"

    def __init__(self, filename):
        self.filename = filename
        self.df_channels = None
        self.df_events = None
        self.read_file()

    def read_file(self):
        channel_map = {}
        event_data = defaultdict(lambda: {"time_tag": None, "tsamp": None, "start_index": None, "waveforms": []})

        with open(self.filename, "rb") as f:
            once = 0
            while True:
                tag = f.read(self.TAG_SIZE)
                if not tag:
                    break

                if tag == self.EVT_TAG:
                    version_bytes = f.read(self.FORMAT_VERSION_SIZE)
                    version = struct.unpack("<i", version_bytes)[0]
                    if (once == 0):
                        print ("Format Version = ",version)
                        once = 1

                elif tag == self.CH_TAG:
                    if (version==1):   # This part weas removed at version 2, redundant
                        version_bytes = f.read(self.FORMAT_VERSION_SIZE)
                        version = struct.unpack("<i", version_bytes)[0]
                        tag2 = f.read(self.TAG_SIZE)
                        if tag2 != self.CH_TAG:
                            raise ValueError(f"Expected CH__STA, got {tag2}")


                    size_bytes = f.read(4)
                    channel_data_size = struct.unpack("<i", size_bytes)[0]

                    header_bytes = f.read(struct.calcsize(self.CHANNEL_HEADER_FORMAT))
                    e, time_tag, tsamp, start_index, g, c, ch = struct.unpack(self.CHANNEL_HEADER_FORMAT, header_bytes)

                    extra_bytes = f.read(struct.calcsize(self.EXTRA_FORMAT))
                    name_raw, pmt_ch = struct.unpack(self.EXTRA_FORMAT, extra_bytes)
                    name = name_raw.decode('utf-8').rstrip('\x00')

                    waveform_bytes = f.read(self.TIME_SAMPLES * self.FLOAT_SIZE)
                    waveform = struct.unpack(f"<{self.TIME_SAMPLES}f", waveform_bytes)

                    if ch not in channel_map:
                        channel_map[ch] = {
                            "pmt_ch": pmt_ch,
                            "name": name,
                            "logic_ch": ch,
                            "phys_ch": c,
                            "group": g
                        }

                    ed = event_data[e]
                    ed["time_tag"] = time_tag
                    ed["tsamp"] = tsamp
                    ed["start_index"] = start_index
                    ed["waveforms"].append({
                        "logic_ch": ch,
                        "waveform": list(waveform)
                    })

                else:
                    raise ValueError(f"Unknown tag found: {tag}")

        self.df_channels = pd.DataFrame.from_dict(channel_map, orient='index')

        events_list = []
        for e, data in event_data.items():
            events_list.append({
                "event": e,
                "time_tag": data["time_tag"],
                "tsamp": data["tsamp"],
                "start_index": data["start_index"],
                "waveforms": data["waveforms"]
            })
        self.df_events = pd.DataFrame(events_list)

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
                info = self.df_channels[self.df_channels['logic_ch'] == logic_ch].iloc[0]
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
    reader = DX2("out.DXM")
    print("Channels:")
    print(reader.df_channels.head())
    print("\nEvents:")
    print(reader.df_events.head())

    reader.plot_event_waveforms(0, separate_subplots=False)
    reader.draw_summary()
