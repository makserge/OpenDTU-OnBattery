import os
import subprocess
from SCons.Script import Import

Import("env")

# Configuration - Match these to your manage_ramdisk.sh
RAMDISK_PATH = "/Volumes/PIO_RAM/esp32p4_build"
BACKUP_PATH = os.path.expanduser("~/.pio_ram_backup/esp32p4_build")

def sync_to_ssd(source, target, env):
    print("\n--- [POST-BUILD] Syncing RAM Disk to SSD Backup ---")
    
    # Ensure backup directory exists
    if not os.path.exists(BACKUP_PATH):
        os.makedirs(BACKUP_PATH)

    # Use rsync for high-speed differential backup
    try:
        subprocess.run([
            "rsync", "-av", "--delete", 
            RAMDISK_PATH + "/", 
            BACKUP_PATH + "/"
        ], check=True)
        print("--- [POST-BUILD] Sync Complete! ---")
    except Exception as e:
        print(f"--- [POST-BUILD] Sync Failed: {e} ---")

# Register the function to run after the 'buildprog' (the .elf/.bin creation)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", sync_to_ssd)
