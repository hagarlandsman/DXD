from DX2FileReader import *

reader = DX2("/home/hagar/Workspace/codes/DXD/example/out_with_correction.DX2")
print("Channels:")
print(reader.df_channels.head())

print("\nEvents:")
print(reader.df_events.head())

reader.plot_event_waveforms(0, separate_subplots=False)
reader.draw_summary()
