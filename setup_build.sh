#!/bin/bash
# Check if RAM disk is already mounted
if [ ! -d "/Volumes/PIO_RAM" ]; then
    echo "Creating 8GB RAM Disk..."
    diskutil erasevolume HFS+ 'PIO_RAM' `hdiutil attach -nomount ram://16777216`
fi

# Create the specific project build directory
mkdir -p /Volumes/PIO_RAM/esp32p4_build
echo "RAM Disk ready for PlatformIO."