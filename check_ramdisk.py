import os
import shutil
import subprocess

RAMDISK_PATH = "/Volumes/PIO_RAM"
THRESHOLD_PERCENT = 80

def send_notification(message, title="PlatformIO RAMDisk"):
    # Triggers a native macOS notification with a sound
    cmd = f'display notification "{message}" with title "{title}" sound name "Glass"'
    subprocess.call(["osascript", "-e", cmd])

if os.path.exists(RAMDISK_PATH):
    # Get disk usage stats
    usage = shutil.disk_usage(RAMDISK_PATH)
    percent_used = (usage.used / usage.total) * 100
    
    print(f"--- [RAMDISK] Usage: {percent_used:.1f}% ---")
    
    if percent_used > THRESHOLD_PERCENT:
        msg = f"Warning: RAM disk is {percent_used:.1f}% full. Consider clearing old builds."
        print(f"!!! {msg} !!!")
        send_notification(msg)
else:
    print(f"ERROR: RAM disk NOT found at {RAMDISK_PATH}")
    # (Existing logic to abort build if disk is missing)
