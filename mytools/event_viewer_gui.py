import sys
import os

import tkinter as tk
from tkinter import ttk, filedialog
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from DX2FileReader import DX2




class EventViewerApp:
    def __init__(self, master, reader: DX2):
        self.master = master
        self.reader = reader
        self.current_event = 0
        self.use_subplots = False
        self.fig = None

        self.master.title("Waveform Event Viewer")
        self.create_widgets()
        self.plot_event()

    def create_widgets(self):
        self.plot_frame = tk.Frame(self.master)
        self.plot_frame.pack(fill=tk.BOTH, expand=True)

        self.button_frame = tk.Frame(self.master)
        self.button_frame.pack(fill=tk.X)

        # Define button style
        style = ttk.Style()
        style.theme_use("default")
        style.configure("Large.TButton", font=("Helvetica", 12), padding=10, foreground="white", background="#007acc")
        style.map("Large.TButton", background=[("active", "#005f99")])

        # Create buttons
        self.prev_button = ttk.Button(self.button_frame, text="Previous", command=self.prev_event, style="Large.TButton")
        self.prev_button.pack(side=tk.LEFT, padx=5, pady=5)

        self.next_button = ttk.Button(self.button_frame, text="Next", command=self.next_event, style="Large.TButton")
        self.next_button.pack(side=tk.LEFT, padx=5, pady=5)

        self.jump_label = ttk.Label(self.button_frame, text="Go to event:", font=("Helvetica", 12))
        self.jump_label.pack(side=tk.LEFT, padx=5)

        self.jump_entry = ttk.Entry(self.button_frame, width=6, font=("Helvetica", 12))
        self.jump_entry.pack(side=tk.LEFT)

        self.jump_button = ttk.Button(self.button_frame, text="Go", command=self.jump_to_event, style="Large.TButton")
        self.jump_button.pack(side=tk.LEFT, padx=5)

        self.mode_button = ttk.Button(self.button_frame, text="Toggle View Mode", command=self.toggle_mode, style="Large.TButton")
        self.mode_button.pack(side=tk.LEFT, padx=5)

        self.save_csv_button = ttk.Button(self.button_frame, text="Save CSV", command=self.save_csv, style="Large.TButton")
        self.save_csv_button.pack(side=tk.LEFT, padx=5)

        self.save_png_button = ttk.Button(self.button_frame, text="Save PNG", command=self.save_png, style="Large.TButton")
        self.save_png_button.pack(side=tk.LEFT, padx=5)

        self.quit_button = ttk.Button(self.button_frame, text="Quit", command=self.master.quit, style="Large.TButton")
        self.quit_button.pack(side=tk.RIGHT, padx=5)

    def plot_event(self):
        for widget in self.plot_frame.winfo_children():
            widget.destroy()
        self.fig = plt.Figure(figsize=(12, 10), dpi=100)
        event_row = self.reader.df_events[self.reader.df_events['event'] == self.current_event]
        if not event_row.empty:
            row = event_row.iloc[0]
            info_text = (f"File: {self.reader.filename}  |  Event: {self.current_event}  |  "
                         f"TimeTag: {row['time_tag']}  |  Tsamp: {row['tsamp']}  |  StartIndex: {row['start_index']}")
        else:
            info_text = f"File: {self.reader.filename}  |  Event: {self.current_event}"
        self.fig.text(0.01, 0.98, info_text, fontsize=9, va='top')
        event_row = self.reader.df_events[self.reader.df_events['event'] == self.current_event]
        channel_colors = plt.cm.tab20(np.linspace(0, 1, 32))

        if event_row.empty:
            ax = self.fig.add_subplot(111)
            ax.set_title(f"Event {self.current_event} not found")
        else:
            waveforms = event_row.iloc[0]['waveforms']
            tsamp = event_row.iloc[0]['tsamp']
            start_index = event_row.iloc[0]['start_index']

            if self.use_subplots:
                nrows, ncols = 5, 6
                axs = self.fig.subplots(nrows, ncols, sharex='col')
                axs = axs.flatten()
                for i, wf in enumerate(waveforms):
                    logic_ch = wf['logic_ch']
                    y = wf['waveform']
#                    x = [start_index + j * tsamp for j in range(len(y))]
                    x = [j * tsamp  for j in range(len(y))]  # in ns

                    color = channel_colors[logic_ch % len(channel_colors)]
                    info = self.reader.df_channels[self.reader.df_channels['logic_ch'] == logic_ch].iloc[0]
                    axs[i].plot(x, y, color=color)
                    axs[i].set_xlabel("ns")
                    axs[i].set_title(f"Ch {logic_ch}: {info['name']}", fontsize=8)
                    axs[i].grid(True)
                for j in range(len(waveforms), nrows * ncols):
                    axs[j].axis('off')
                self.fig.tight_layout(pad=1.0)

            else:
                ax = self.fig.add_subplot(111)
                legend_data = []
                for wf in waveforms:
                    logic_ch = wf['logic_ch']
                    y = wf['waveform']
#                    x = [start_index + j * tsamp for j in range(len(y))]
                    x = [j * tsamp  for j in range(len(y))]  # in ns

                    color = channel_colors[logic_ch % len(channel_colors)]
                    info = self.reader.df_channels[self.reader.df_channels['logic_ch'] == logic_ch].iloc[0]
                    label = f"Ch {logic_ch}"
                    ax.plot(x, y, label=label, color=color)
                    legend_data.append((logic_ch, info['name'], info['pmt_ch'], info['phys_ch'], info['group']))
                ax.set_title(f"Waveforms for Event {self.current_event}")
                ax.set_xlabel("Time [ns]")
                ax.set_ylabel("Amplitude")
                ax.grid(True)
                ax.legend(loc='upper right', fontsize=8)

                col_labels = ["Logic Ch", "Name", "PMT", "Phys Ch", "Group"]
                table_data = [[str(v) for v in row] for row in legend_data]
                half = (len(table_data) + 1) // 2
                table1 = ax.table(cellText=table_data[:half], colLabels=col_labels, loc='lower center', cellLoc='center', bbox=[0, -0.45, 0.45, 0.4])
                table2 = ax.table(cellText=table_data[half:], colLabels=col_labels, loc='lower center', cellLoc='center', bbox=[0.55, -0.45, 0.45, 0.4])
                for table in [table1, table2]:
                    table.auto_set_font_size(False)
                    table.set_fontsize(8)
                    table.scale(1, 3)
                self.fig.subplots_adjust(bottom=0.35)


        self.canvas = FigureCanvasTkAgg(self.fig, master=self.plot_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def prev_event(self):
        if self.current_event > 0:
            self.current_event -= 1
            self.plot_event()

    def next_event(self):
        if self.current_event < self.reader.df_events['event'].max():
            self.current_event += 1
            self.plot_event()

    def jump_to_event(self):
        try:
            e = int(self.jump_entry.get())
            if e in self.reader.df_events['event'].values:
                self.current_event = e
                self.plot_event()
            else:
                print(f"Event {e} not found.")
        except ValueError:
            print("Invalid event number.")

    def toggle_mode(self):
        self.use_subplots = not self.use_subplots
        self.plot_event()

    def save_csv_fancy(self):
        filename = filedialog.asksaveasfilename(defaultextension=".csv", filetypes=[("CSV files", "*.csv")])
        if filename:
            event_row = self.reader.df_events[self.reader.df_events['event'] == self.current_event]
            if not event_row.empty:
                rows = []
                for wf in event_row.iloc[0]['waveforms']:
                    logic_ch = wf['logic_ch']
                    for i, val in enumerate(wf['waveform']):
                        rows.append({"logic_ch": logic_ch, "sample": i, "value": val})
                df = pd.DataFrame(rows)
                df.to_csv(filename, index=False)
                print(f"Saved to {filename}")

    def save_png_fancy(self):
        filename = filedialog.asksaveasfilename(defaultextension=".png", filetypes=[("PNG files", "*.png")])
        if filename and self.fig:
            self.fig.savefig(filename)
            print(f"Saved image to {filename}")

    def save_csv(self):
        base = self.reader.filename.rsplit(".", 1)[0]
        filename = f"{base}_event{self.current_event:04d}.csv"
        event_row = self.reader.df_events[self.reader.df_events['event'] == self.current_event]
        if not event_row.empty:
            rows = []
            for wf in event_row.iloc[0]['waveforms']:
                logic_ch = wf['logic_ch']
                for i, val in enumerate(wf['waveform']):
                    rows.append({"logic_ch": logic_ch, "sample": i, "value": val})
            df = pd.DataFrame(rows)
            df.to_csv(filename, index=False)
            print(f"Saved to {filename}")

    def save_png(self):
        base = self.reader.filename.rsplit(".", 1)[0]
        filename = f"{base}_event{self.current_event:04d}.png"
        if self.fig:
            self.fig.savefig(filename)
            print(f"Saved image to {filename}")


if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python event_viewer_gui.py your_file.DX2")
    else:
        reader = DX2(sys.argv[1])
        root = tk.Tk()
        app = EventViewerApp(root, reader)
        root.mainloop()
