from DX2FileReader import *

reader = DX2("/home/hagar/Workspace/codes/DXD/example/out1.DX2")

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

