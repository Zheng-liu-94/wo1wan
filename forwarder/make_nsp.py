#!/usr/bin/env python3
"""
Build an ExeFS PFS0 (.nsp) forwarder for Atmosphere.

Atmosphere's NSP installer (DBI / Tinfoil / Tesla menu) accepts an ExeFS PFS0
forwarder WITHOUT a Nintendo signature, so we construct the PFS0 container
directly here. No hacbrewpack / prod.keys needed.

Expected layout (relative to CWD):
  exefs/main                     -> the .nso (forwarder code)
  exefs/main.npdm               -> NPDM (application_type=1)
  control/control.nacp          -> NACP metadata
  control/icon_AmericanEnglish.dat -> 256x256 JPEG icon

Output: wo1wan.nsp
"""
import struct
import os
import sys

FILES = [
    ("exefs/main", "exefs/main"),
    ("exefs/main.npdm", "exefs/main.npdm"),
    ("control/control.nacp", "control/control.nacp"),
    ("control/icon_AmericanEnglish.dat", "control/icon_AmericanEnglish.dat"),
]


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "wo1wan.nsp"

    for _arc, loc in FILES:
        if not os.path.exists(loc):
            sys.stderr.write(f"ERROR: missing input file: {loc}\n")
            sys.exit(1)

    # String table: concatenated null-terminated names.
    string_table = b""
    entries = []
    for arc, loc in FILES:
        name = arc.encode("utf-8") + b"\x00"
        offset = len(string_table)
        string_table += name
        entries.append((offset, loc))

    num_files = len(FILES)
    string_table_size = len(string_table)
    header_size = 0x10 + 0x18 * num_files + string_table_size

    # Canonical PFS0 header (MUST be 16 bytes): magic + num_files + string_table_size + reserved.
    # Atmosphere's NSP installer reads num_files at offset 0x04; an extra "version" field here
    # would shift it to 0x08 and make the installer see 0 files -> install fails.
    header = b"PFS0"
    header += struct.pack("<I", num_files)     # num_files
    header += struct.pack("<I", string_table_size)
    header += struct.pack("<I", 0)            # reserved

    file_entries = b""
    file_data = b""
    cur_offset = 0
    for str_off, loc in entries:
        data = open(loc, "rb").read()
        size = len(data)
        file_entries += struct.pack("<Q", cur_offset)   # data_offset (rel)
        file_entries += struct.pack("<Q", size)
        file_entries += struct.pack("<I", str_off)
        file_entries += struct.pack("<I", 0)
        file_data += data
        pad = (0x10 - (size % 0x10)) % 0x10
        file_data += b"\x00" * pad
        cur_offset += size + pad

    pfs0 = header + file_entries + string_table + file_data

    with open(out_path, "wb") as f:
        f.write(pfs0)

    print(f"Wrote {out_path}: {len(pfs0)} bytes "
          f"({num_files} files)")


if __name__ == "__main__":
    main()
