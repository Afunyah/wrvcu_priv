import serial
import serial.tools.list_ports
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

PRODUCT_ID = "0483"  # Teensy USB Product ID

HEADER = "\033[95m"
OKBLUE = "\033[94m"
OKCYAN = "\033[96m"
OKGREEN = "\033[92m"
WARNING = "\033[93m"
FAIL = "\033[91m"
ENDC = "\033[0m"
BOLD = "\033[1m"
UNDERLINE = "\033[4m"


log_strs = ["DEBUG", "INFO", "WARNING", "ERROR"]
colours = [OKGREEN, OKBLUE, WARNING, FAIL]


def print_line(line: bytes):
    decoded = line.decode("ascii").strip()

    # Is this part of a stack trace?
    if "#" in decoded and "0x" in decoded:
        print(decoded, end="\t\t")

        # Decode the address into line numbers and print

        # Trace may contain some junk so split it
        trace = decoded.split()

        for i, s in enumerate(trace):
            # Find first hex value
            if "0x" in s:
                addr = trace[i]
                res = subprocess.run(
                    [str(ADDR2LINE), "-e", str(ELF), "-pfCs", addr], capture_output=True
                )
                print(OKBLUE + res.stdout.decode().strip() + ENDC)
                break

    elif any(x in decoded for x in log_strs):
        log_str = None
        colour = None
        for i, x in enumerate(log_strs):
            if x in decoded:
                log_str = x
                colour = colours[i]

        l, m, r = decoded.partition(log_str)
        print(l + colour + m + ENDC + r)

    else:
        print(decoded)


def main():
    port = None

    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if PRODUCT_ID in p[2]:
            port = p[0]

    if port is None:
        print("Cannot find Teensy!")
        return
    else:
        print(f"Opening port {port}")

    while True:
        try:
            with serial.Serial(port, baudrate=9600) as ser:
                while True:
                    try:
                        if ser.inWaiting() > 0:
                            print_line(ser.readline())
                    except KeyboardInterrupt:
                        raise
                    except Exception:
                        break  # reopen on failure
        except KeyboardInterrupt:
            raise
        except:
            pass


if __name__ == "__main__":
    main()
