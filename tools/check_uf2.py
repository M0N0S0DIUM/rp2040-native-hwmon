#!/usr/bin/env python3
"""Validate Pico RP2040 UF2 block structure and ROM boot2 CRC, independent of SDK."""
import argparse
import binascii
from pathlib import Path
import struct

BLOCK_BYTES = 512
MAGIC_START_0 = 0x0A324655
MAGIC_START_1 = 0x9E5D5157
MAGIC_END = 0x0AB16F30
FAMILY_RP2040 = 0xE48BFF56

def bit_reverse(value: int, width: int) -> int:
    return int(f"{value:0{width}b}"[::-1], 2)

def check(path: Path) -> None:
    raw = path.read_bytes()
    assert len(raw) >= 512 and len(raw) % BLOCK_BYTES == 0, "Bad UF2 file size"
    pages = []
    total = len(raw) // BLOCK_BYTES
    for i in range(total):
        block = raw[512*i:512*(i+1)]
        m0, m1, flags, addr, size, index, count, family = struct.unpack_from("<8I", block)
        assert (m0, m1) == (MAGIC_START_0, MAGIC_START_1), f"Bad UF2 magic at {i}"
        assert struct.unpack_from("<I", block, 508)[0] == MAGIC_END, f"Bad UF2 footer at {i}"
        assert (flags & 0x2000) and family == FAMILY_RP2040, f"Wrong RP2040 family at {i}"
        assert index == i and count == total, f"Wrong block index/count at {i}"
        assert 0 < size <= 476, f"Bad payload size at {i}"
        assert addr == 0x10000000 + 256*i and size == 256, f"Noncontiguous flash at {i}"
        pages.append(block[32:32 + size])
    image = b"".join(pages)
    stage2, stored = image[:252], struct.unpack_from("<I", image, 252)[0]
    actual = bit_reverse(
        binascii.crc32(bytes(bit_reverse(b, 8) for b in stage2)) ^ 0xFFFFFFFF,
        32,
    )
    assert actual == stored, f"RP2040 boot2 checksum incorrect: {stored:#x} != {actual:#x}"
    stack, reset = struct.unpack_from("<II", image, 256)
    assert 0x20000000 <= stack < 0x20042000, f"Unexpected stack pointer: {stack:#x}"
    assert 0x10000100 <= (reset & ~1) < 0x10100000 and reset & 1, f"Bad reset handler: {reset:#x}"
    print(f"PASS {path}: {total} UF2 blocks, correct RP2040 boot2 CRC {actual:#x}, "
          f"reset {reset:#x}, {len(image)} flash bytes")

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("files", type=Path, nargs="+")
    args = p.parse_args()
    for f in args.files:
        check(f)
