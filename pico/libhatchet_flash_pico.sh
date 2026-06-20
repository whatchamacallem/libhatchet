#!/usr/bin/env bash
# Build the libhatchet test suite for Pico 2, flash it, then stream serial output.
# Run with: ./flash_libhatchet.sh [--port /dev/ttyACM0] [--board pico2]
set -euo pipefail

PICO_ROOT="$(cd "$(dirname "$0")" && pwd)"
PICO_SDK_PATH="$PICO_ROOT/pico-sdk"
BUILD_DIR="$PICO_ROOT/build-hatchet"
BOARD="${BOARD:-pico2}"
BAUD="${BAUD:-115200}"
PORT="${PORT:-}"

export PICO_SDK_PATH
export LIBHATCHET_PATH="../libhatchet"

# ── argument parsing ───────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --port)  PORT="$2";  shift 2 ;;
        --board) BOARD="$2"; shift 2 ;;
        --baud)  BAUD="$2";  shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# ── helpers ────────────────────────────────────────────────────────────────────
info()  { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
ok()    { echo -e "\033[1;32m[OK]\033[0m    $*"; }
warn()  { echo -e "\033[1;33m[WARN]\033[0m  $*"; }
die()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; exit 1; }

require() { command -v "$1" &>/dev/null || die "Required tool not found: $1"; }

# ── pre-flight checks ──────────────────────────────────────────────────────────
require cmake
require arm-none-eabi-gcc
require picotool

# ── 1. build ───────────────────────────────────────────────────────────────────
info "Configuring CMake for board '$BOARD' ..."
cmake -S "$PICO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DPICO_BOARD="$BOARD" \
    -DPICO_SDK_PATH="$PICO_SDK_PATH" \
    -G Ninja \
    -Wno-dev \
    2>&1

info "Building libhatchet_test_pico ..."
cmake --build "$BUILD_DIR" --target libhatchet_test_pico -j"$(nproc)"

UF2="$BUILD_DIR/libhatchet_test_pico.uf2"
[[ -f "$UF2" ]] || die "Build succeeded but $UF2 not found."
ok "Build complete: $UF2"

# ── 2. flash ───────────────────────────────────────────────────────────────────
info "Looking for Pico 2 in BOOTSEL mode ..."

TIMEOUT=30
ELAPSED=0
while ! picotool info &>/dev/null; do
    if [[ $ELAPSED -eq 0 ]]; then
        warn "No Pico found in BOOTSEL mode."
        echo ""
        echo "  ► Hold the BOOTSEL button on the Pico 2, plug in USB, then release."
        echo "  ► Waiting up to ${TIMEOUT}s ..."
        echo ""
    fi
    sleep 1
    (( ELAPSED++ ))
    if [[ $ELAPSED -ge $TIMEOUT ]]; then
        die "Timed out waiting for Pico in BOOTSEL mode."
    fi
done

ok "Pico detected. Flashing ..."
picotool load -f "$UF2"
picotool reboot

ok "Flash complete — Pico is rebooting."

# ── 3. find serial port ────────────────────────────────────────────────────────
if [[ -z "$PORT" ]]; then
    info "Waiting for serial port to appear ..."
    for i in $(seq 1 150); do
        for candidate in /dev/ttyACM0 /dev/ttyACM1 /dev/ttyUSB0 /dev/ttyUSB1; do
            if [[ -e "$candidate" ]]; then
                PORT="$candidate"
                break 2
            fi
        done
        sleep 0.1
    done
fi

# Let the driver stabilize so there is something to cat.
sleep 0.5

if [[ -z "$PORT" ]] || [[ ! -e "$PORT" ]]; then
    die "No serial port found. Try: ./flash_hatchet.sh --port /dev/ttyACM0"
fi

ok "Serial port: $PORT at ${BAUD} baud."
echo ""

stty -F "$PORT" "$BAUD" raw -echo -hupcl
cat "$PORT" || true
