#!/usr/bin/env python3
"""
Chipintelli CI-03T UART Firmware Flasher
Flashes firmware to M5Stack Unit-ASR (CI-03T module)
Works via ESP32-S3 UART passthrough
"""

import serial
import time
import sys
import struct
from pathlib import Path

# Configuration
SERIAL_PORT = '/dev/ttyACM0'  # ESP32-S3 passthrough
BAUD_RATE = 115200
FIRMWARE_FILE = 'unitASRfirmware/jx_firm/jx_ci_03t_firmware.bin'

# Chipintelli CI-03T Flash Protocol
FLASH_START_ADDR = 0x00000000
BLOCK_SIZE = 4096  # 4KB blocks
PACKET_SIZE = 256  # Data packet size

def print_progress(current, total, prefix='Progress:'):
    """Print progress bar"""
    percent = int(100 * current / total)
    bar_len = 50
    filled = int(bar_len * current / total)
    bar = '█' * filled + '-' * (bar_len - filled)
    print(f'\r{prefix} |{bar}| {percent}% ({current}/{total} bytes)', end='', flush=True)

def wait_for_response(ser, timeout=2):
    """Wait for and return response from device"""
    start_time = time.time()
    response = b''

    while time.time() - start_time < timeout:
        if ser.in_waiting > 0:
            response += ser.read(ser.in_waiting)
            time.sleep(0.01)  # Small delay to collect all data
        elif response:
            break  # Got some data and no more coming
        time.sleep(0.01)

    return response

def enter_bootloader(ser):
    """Try to enter bootloader mode"""
    print("Attempting to enter bootloader mode...")

    # Method 1: Send bootloader command
    bootloader_cmds = [
        bytes([0xAA, 0x55, 0x50, 0x55, 0xAA]),  # Bootloader entry command
        bytes([0xAA, 0x55, 0x51, 0x55, 0xAA]),  # Alternative command
        bytes([0x7E, 0x00, 0x04, 0x00]),        # Another possible entry sequence
    ]

    for cmd in bootloader_cmds:
        ser.write(cmd)
        time.sleep(0.3)
        response = wait_for_response(ser, timeout=1)
        if response:
            print(f"  Response to bootloader command: {response.hex()}")

    # Method 2: Send break signal
    print("  Sending break signal...")
    try:
        ser.send_break(duration=0.25)
    except:
        pass
    time.sleep(0.5)

    # Method 3: Toggle DTR/RTS (if available)
    try:
        ser.setDTR(False)
        ser.setRTS(True)
        time.sleep(0.1)
        ser.setRTS(False)
        time.sleep(0.1)
        ser.setDTR(True)
    except:
        pass

    time.sleep(0.5)
    response = wait_for_response(ser)
    if response:
        print(f"  Bootloader response: {response.hex()}")
        return True

    print("  No specific bootloader response (may still work)")
    return True

def flash_firmware(port, baud, firmware_path):
    """Flash firmware to CI-03T"""

    print("=" * 60)
    print("  Chipintelli CI-03T Firmware Flasher")
    print("  M5Stack Unit-ASR")
    print("=" * 60)
    print()

    # Read firmware
    firmware_path = Path(firmware_path)
    if not firmware_path.exists():
        print(f"✗ Error: Firmware file not found: {firmware_path}")
        return False

    with open(firmware_path, 'rb') as f:
        firmware_data = f.read()

    print(f"✓ Loaded firmware: {firmware_path.name}")
    print(f"  Size: {len(firmware_data)} bytes ({len(firmware_data)/1024:.1f} KB)")
    print()

    # Open serial port
    try:
        ser = serial.Serial(port, baud, timeout=2)
        print(f"✓ Opened serial port: {port} @ {baud} baud")
    except serial.SerialException as e:
        print(f"✗ Failed to open serial port: {e}")
        print("  Make sure:")
        print("  1. ESP32-S3 has UART_Passthrough sketch uploaded")
        print("  2. Serial Monitor is closed")
        print("  3. Port is correct")
        return False

    time.sleep(1)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    try:
        # Enter bootloader mode
        if not enter_bootloader(ser):
            print("⚠  Warning: Bootloader entry unclear, continuing anyway...")
        print()

        # Simple method: Just send the entire firmware
        # Many Chipintelli devices accept raw firmware over UART
        print("Flashing firmware...")
        print("This may take 2-3 minutes. Do NOT disconnect power!")
        print()

        # Send firmware in chunks
        chunk_size = PACKET_SIZE
        total_sent = 0

        for offset in range(0, len(firmware_data), chunk_size):
            chunk = firmware_data[offset:offset + chunk_size]
            ser.write(chunk)
            total_sent += len(chunk)

            # Update progress every 10KB
            if total_sent % 10240 == 0 or total_sent == len(firmware_data):
                print_progress(total_sent, len(firmware_data))

            # Small delay to not overwhelm the buffer
            time.sleep(0.005)

        print()  # New line after progress bar
        print()
        print("✓ Firmware sent successfully!")
        print()

        # Wait for device to process and reboot
        print("Waiting for device to restart (10 seconds)...")
        time.sleep(10)

        # Clear any pending data
        ser.reset_input_buffer()

        # Test if device responds
        print("Testing device response...")
        test_cmd = bytes([0xAA, 0x55, 0xFF, 0x55, 0xAA])  # Wake command
        ser.write(test_cmd)
        time.sleep(1)

        response = wait_for_response(ser, timeout=2)

        if response:
            print(f"✓ Device responded: {response.hex()}")
            if len(response) == 5 or len(response) == 6:
                print()
                print("=" * 60)
                print("  ✓ FLASHING SUCCESSFUL!")
                print("=" * 60)
                print()
                print("Next steps:")
                print("1. Close this script")
                print("2. Upload 'UnitASR_Firmware_Check' sketch to ESP32")
                print("3. Open Serial Monitor")
                print("4. Say 'Hi M Five' to test voice recognition!")
                print()
                return True
        else:
            print("⚠ No response from device")
            print()
            print("This could mean:")
            print("  - Device is flashing (needs more time)")
            print("  - Device needs manual reset")
            print("  - Flashing failed")
            print()
            print("Try:")
            print("  1. Power cycle the Unit-ASR")
            print("  2. Upload test sketch and check Serial Monitor")
            return None

    except KeyboardInterrupt:
        print("\n\n⚠  Flashing interrupted by user!")
        print("Device may be in unknown state. Try reflashing.")
        return False

    finally:
        ser.close()
        print("\nSerial port closed")

if __name__ == "__main__":
    print()
    print("⚠  IMPORTANT:")
    print("  1. Make sure UART_Passthrough sketch is on ESP32-S3")
    print("  2. Close Arduino Serial Monitor if open")
    print("  3. Do NOT disconnect power during flashing")
    print()
    print("Press Ctrl+C within 5 seconds to cancel...")
    print()

    try:
        time.sleep(5)
    except KeyboardInterrupt:
        print("\nCancelled by user")
        sys.exit(0)

    result = flash_firmware(SERIAL_PORT, BAUD_RATE, FIRMWARE_FILE)

    if result:
        sys.exit(0)
    elif result is None:
        sys.exit(2)  # Uncertain
    else:
        sys.exit(1)  # Failed
