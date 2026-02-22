import subprocess
from SCons.Script import Import

Import("env")

def get_git_hash():
    try:
        # Get the 7-character short hash
        hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()
        return f'\\"{hash}\\"'
    except Exception:
        return '\\"unknown\\"'

# Inject the macro into the build environment
env.Append(CPPDEFINES=[
    ("__COMPILED_GIT_HASH__", get_git_hash())
])

print(f"--- [GIT] Injected Hash: {get_git_hash()} ---")
