import os
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

# Check if ccache is installed
if os.system("ccache --version > /dev/null 2>&1") == 0:
    # Prefix the compiler and linker with ccache
    env.Replace(
        CCACHE="ccache",
        CC=f"ccache {env['CC']}",
        CXX=f"ccache {env['CXX']}",
        AS=f"ccache {env['AS']}"
    )
    print("--- [ccache] Enabled successfully ---")
else:
    print("--- [ccache] Warning: ccache not found in system PATH ---")
