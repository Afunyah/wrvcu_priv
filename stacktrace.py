from pathlib import Path
import subprocess

ADDR2LINE = (
    Path.home()
    / ".platformio"
    / "packages"
    / "toolchain-arm-cortexm-win64"
    / "bin"
    / "arm-cortexm0p-eabi-addr2line"
)

ELF = Path.cwd() / ".pio" / "build" / "teensy41" / "firmware.elf"


def main():
    while True:
        # Trace may contain some junk so split it
        trace = input().split()

        for i, s in enumerate(trace):
            # Find first hex value
            if "0x" in s:
                addr = trace[i]
                subprocess.call(
                    [str(ADDR2LINE), "-e", str(ELF), "-pfCs", addr],
                )
                break


if __name__ == "__main__":
    main()
