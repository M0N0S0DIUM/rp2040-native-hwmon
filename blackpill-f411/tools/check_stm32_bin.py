#!/usr/bin/env python3
"""Basic STM32F411 binary vector-table validation. Not a hardware USB test."""
import pathlib, sys, struct
for name in sys.argv[1:]:
    data=pathlib.Path(name).read_bytes()
    assert 1024 < len(data) < 512*1024, (name,len(data))
    stack, reset=struct.unpack_from("<II", data, 0)
    assert 0x20000000 <= stack <= 0x20020000, (name,hex(stack))
    assert reset & 1 and 0x08000000 <= (reset & ~1) < 0x08080000, (name,hex(reset))
    print(f"PASS {name}: {len(data)} bytes, SP=0x{stack:08x}, reset=0x{reset:08x}")
