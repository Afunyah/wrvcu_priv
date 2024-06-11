import sys
import subprocess
import os
import pathlib

Import("env")

print("Building DBC source files...")
print(sys.executable)
try:
    import cantools
except ImportError:
    subprocess.check_call([sys.executable, "-m", "pip", "install", "cantools"])

for file_name in pathlib.Path("dbc").glob("*.dbc"):
    subprocess.call(
        [
            sys.executable,
            "-m",
            "cantools",
            "generate_c_source",
            "--prune",
            "--use-float",
            "-o",
            ".dbc_gen",
            file_name,
        ]
    )

env.BuildSources(
    os.path.join("$BUILD_DIR", ".dbc_gen"),
    os.path.join("$PROJECT_DIR", ".dbc_gen"),
)

env.Append(
    CPPPATH=[".dbc_gen"],
)
