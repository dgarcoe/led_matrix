#!/usr/bin/env python3
"""BLE client to control the DeskStatus LED matrix.

Usage:
    python set_status.py green          # Available
    python set_status.py yellow         # Busy but interruptible
    python set_status.py red            # Do Not Disturb
    python set_status.py off            # LEDs off
    python set_status.py --address AA:BB:CC:DD:EE:FF green  # Use specific MAC

Requires: pip install bleak
"""

import argparse
import asyncio
import sys

from bleak import BleakClient, BleakScanner

DEVICE_NAME = "DeskStatus"
STATUS_CHAR_UUID = "12345678-1234-5678-1234-56789abcdef1"

STATUS_MAP = {
    "off":    b"\x00",
    "green":  b"\x01",
    "yellow": b"\x02",
    "red":    b"\x03",
}


async def find_device(name: str, timeout: float = 10.0):
    """Scan for a BLE device by name."""
    print(f"Scanning for '{name}'...")
    device = await BleakScanner.find_device_by_name(name, timeout=timeout)
    if device is None:
        print(f"Error: Could not find device '{name}'. Is it powered on?")
        sys.exit(1)
    print(f"Found: {device.name} ({device.address})")
    return device.address


async def set_status(address: str, status: str):
    """Connect to the device and set the status."""
    value = STATUS_MAP[status]
    print(f"Connecting to {address}...")
    async with BleakClient(address) as client:
        print(f"Connected. Setting status to '{status}'...")
        await client.write_gatt_char(STATUS_CHAR_UUID, value)
        print("Done.")


async def main():
    parser = argparse.ArgumentParser(description="Set DeskStatus LED color via BLE")
    parser.add_argument("status", choices=STATUS_MAP.keys(), help="Status to set")
    parser.add_argument("--address", "-a", help="BLE MAC address (skips scan)")
    args = parser.parse_args()

    address = args.address or await find_device(DEVICE_NAME)
    await set_status(address, args.status)


if __name__ == "__main__":
    asyncio.run(main())
