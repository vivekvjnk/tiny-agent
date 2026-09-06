#!/usr/bin/env bash
# rpiboot_flash.sh - Helper script to flash Raspberry Pi 4 EEPROM over USB-C using rpiboot
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RECOVERY_DIR="${SCRIPT_DIR}/../references/recovery_payload"
RPIBOOT_BIN="${SCRIPT_DIR}/rpiboot"

# Check if rpiboot exists or build it if missing
if [ ! -f "${RPIBOOT_BIN}" ]; then
    if command -v rpiboot &>/dev/null; then
        RPIBOOT_BIN="$(command -v rpiboot)"
    else
        echo "[*] Compiling rpiboot from /tmp/usbboot..."
        if [ ! -d "/tmp/usbboot" ]; then
            git clone --depth 1 https://github.com/raspberrypi/usbboot.git /tmp/usbboot
        fi
        (cd /tmp/usbboot && make)
        RPIBOOT_BIN="/tmp/usbboot/rpiboot"
    fi
fi

# Ensure recovery directory exists
if [ ! -d "${RECOVERY_DIR}" ]; then
    mkdir -p "${RECOVERY_DIR}"
fi

# Ensure bootcode4.bin symlink exists in recovery dir
if [ -f "${RECOVERY_DIR}/recovery.bin" ] && [ ! -f "${RECOVERY_DIR}/bootcode4.bin" ]; then
    ln -sf recovery.bin "${RECOVERY_DIR}/bootcode4.bin"
fi

# Ensure pieeprom.upd exists
if [ -f "${RECOVERY_DIR}/pieeprom.bin" ] && [ ! -f "${RECOVERY_DIR}/pieeprom.upd" ]; then
    cp "${RECOVERY_DIR}/pieeprom.bin" "${RECOVERY_DIR}/pieeprom.upd"
fi

# Ensure pieeprom.sig.upd exists
if [ -f "${RECOVERY_DIR}/pieeprom.sig" ] && [ ! -f "${RECOVERY_DIR}/pieeprom.sig.upd" ]; then
    cp "${RECOVERY_DIR}/pieeprom.sig" "${RECOVERY_DIR}/pieeprom.sig.upd"
fi

echo "========================================================="
echo "Raspberry Pi 4 EEPROM Direct USB-C Flash Utility"
echo "========================================================="
echo "Instructions:"
echo " 1. Unplug power and REMOVE MicroSD card from the Pi 4B."
echo " 2. Connect USB-C cable from Host PC directly to Pi 4B USB-C port."
echo " 3. Running rpiboot listener now..."
echo "========================================================="

sudo "${RPIBOOT_BIN}" -v -d "${RECOVERY_DIR}"
