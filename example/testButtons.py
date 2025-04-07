import tkinter as tk
from tkinter import ttk

root = tk.Tk()
root.title("Styled Buttons")

# Configure a ttk style
style = ttk.Style()
style.theme_use("default")  # use a theme that allows color changes

style.configure(
    "Large.TButton",
    font=("Helvetica", 14),
    padding=10,
    foreground="white",
    background="#007acc"
)
style.map("Large.TButton",
          background=[("active", "#005f99")])

# Create a frame
frame = ttk.Frame(root, padding=10)
frame.pack(fill=tk.BOTH, expand=True)

# Styled buttons
ttk.Button(frame, text="Previous", style="Large.TButton").pack(side=tk.LEFT, padx=10)
ttk.Button(frame, text="Next", style="Large.TButton").pack(side=tk.LEFT, padx=10)
ttk.Button(frame, text="Quit", style="Large.TButton", command=root.quit).pack(side=tk.RIGHT, padx=10)

root.mainloop()
