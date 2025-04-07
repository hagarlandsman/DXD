
import subprocess
import sys
import os
from pathlib import Path

# Usage: python run_and_view.py calibration_dir setup_file input_file

if len(sys.argv) != 4:
    print("Usage: python run_and_view.py calibration_dir setup_file input_file")
    sys.exit(1)

cwd = str(Path.cwd())+"/"
calibration_dir = cwd + sys.argv[1]
setup_file = cwd + sys.argv[2]
input_file = cwd + sys.argv[3]

output_prefix = os.path.splitext(input_file)[0]
output_file = output_prefix # + ".DX2"

# Run the C++ extractor
extract_cmd = ["../src/DxDataExtractor", calibration_dir + "/ ", setup_file, input_file, output_prefix]
print("Running extractor:", " ".join(extract_cmd))
subprocess.run(extract_cmd, check=True)

# Launch the GUI
subprocess.run(["python3", "event_viewer_gui.py", output_file])
