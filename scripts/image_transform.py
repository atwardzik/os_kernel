#!/usr/bin/env python
import sys
import os

width = 0
height = 0
max_val = 0

image_map = []
transformed_map = []

transformed_codes = (0x00, 0x55, 0xAA, 0xFF)
# transformed_codes = (0x00, 0xFF)

header = ""

if len(sys.argv) < 2 or sys.argv[1] == "--h":
    print("""

image_transform.sh is a script that transforms given ppm image to the format
accepted by GeT OS. It stripps size information and transforms colors to
2-bits encoding for each color, aligning each to the nearest value among
{0x00, 0x55, 0xaa, 0xff}. The output format for each pixel is: 0x00bbggrr

    """)
    print("\n\tUsage: ./image_transform.py <ppm_file>")
    sys.exit()

filename = sys.argv[1]
with open(filename, "rb") as file:
    fmt = file.readline().strip().decode()
    # width = int(file.readline().strip())
    # height = int(file.readline().strip())
    dims = file.readline().strip()
    width, height = map(int, dims.split())
    max_val = int(file.readline().strip())

    image_map = file.read(width * height * 3)

header = f"{fmt}\n{width}\n{height}\n{max_val}\n"
print(header)

for i, byte in enumerate(image_map):
    calculated_abs = []
    for code in transformed_codes:
        calculated_abs.append(abs(byte - code))

    transformed_map.append(transformed_codes[calculated_abs.index(min(calculated_abs))])

"""
for i, val in enumerate(image_map):
    print(f"{val:02x}", end=" ")

    if ((i + 1) % 3) == 0:
        print()
"""

new_file_path = "../images/"
directory, file_with_ext = os.path.split(filename)
new_filename = file_with_ext[:file_with_ext.rfind(".")]

with open(new_file_path + new_filename + ".ppm", "wb") as file:
    file.write(str.encode(header))

    for byte in transformed_map:
        file.write(byte.to_bytes(1, byteorder="big"))

for i in range(len(transformed_map)):
    byte = transformed_map[i]
    if byte == 0x55:
        transformed_map[i] = 1
    elif byte == 0xaa:
        transformed_map[i] = 2
    elif byte == 0xff:
        transformed_map[i] = 3
    else:
        transformed_map[i] = 0


with open(new_file_path + new_filename + ".raw", "wb") as file:
    i = 0
    current_byte = 0
    while i < len(transformed_map):
        current_byte = (transformed_map[i + 2] << 4) | (transformed_map[i + 1] << 2) | (transformed_map[i])
        file.write(current_byte.to_bytes(1, byteorder="big"))
        i += 3
