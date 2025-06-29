import tkinter as tk
import subprocess
import threading
import time

def get_nvidia_smi_output():
    try:
        result = subprocess.check_output(["nvidia-smi"], encoding="utf-8")
        return result
    except FileNotFoundError:
        return "nvidia-smi not found.  Ensure drivers are installed."
    except Exception as e:
        return f"Error: {e}"

def update_gui():
    output = get_nvidia_smi_output()
    text_area.delete("0.0", tk.END)  # Clear existing text
    text_area.insert(tk.END, output)

    root.after(2000, update_gui) # Schedule next update after 2 seconds (adjust as needed)


root = tk.Tk()
root.title("NVIDIA-SMI Monitor")

text_area = tk.Text(root, height=30, width=100)  # Adjust size as needed
text_area.pack()

update_gui() # Start the initial update

root.mainloop()
