# Example1.py

import DX2FileReader as DX2
data = DX2.DX2("out_0001.V3.DX2")

print (f"Got {data.Nevents}")
Ntot = int(data.Nevents)

for n in range(1,Ntot):
    ev = data.read_event(n)
    Nch = len(ev['waveforms'])
    print (f"Event {n} : Sampling at {1/ev['tsamp']} GHz, time tag {ev['time_tag']}, got {Nch} channels ")

    for xch in range(0, Nch):
        wf=ev['waveforms'][xch]
        ch_name = wf['name']
        ch_digi = wf['pmt_ch']
        ch_id = wf['logic_ch']
        wf_data = wf['waveform']
        print (f"\t Channel {xch}: {ch_name} at Digitizer={ch_digi} has {len(wf_data)} samples, with max {max(wf_data)}, and min {min(wf_data)}")
    if n==10:
        break

