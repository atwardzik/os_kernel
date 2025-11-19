#!/usr/bin/env python
import subprocess
import re
import sys
from pathlib import Path

def extract_mnemonics(binary_path):
    if not Path(binary_path).exists():
        print(f"Error: File not found: {binary_path}")
        sys.exit(1)

    try:
        result = subprocess.run(
            ["arm-none-eabi-objdump", "-d", binary_path],
            capture_output=True,
            text=True,
            check=True
        )
    except subprocess.CalledProcessError as e:
        print("Error running objdump:", e)
        sys.exit(1)

    disasm = result.stdout
    mnemonics = set()

    for line in disasm.splitlines():
        m = re.match(r"([0-9]|[a-f])+:\s+(([0-9]|[a-f]){4}(\s([0-9]|[a-f])+)?)\s+([a-zA-Z0-9.]+)", line)
        if m:
            mnemonic = m.group(6).strip()
            if mnemonic == ".short" or mnemonic == "8000":
                print(line)
            mnemonics.add(mnemonic)

    return sorted(mnemonics)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <binary>")
        sys.exit(1)

    binary = sys.argv[1]
    mnemonics = extract_mnemonics(binary)

    print(f"\nFound {len(mnemonics)} unique mnemonics in {binary}:\n")
    for m in mnemonics:
        print(m)
