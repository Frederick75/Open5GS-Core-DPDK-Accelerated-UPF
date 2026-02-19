#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j

echo "[*] Starting UPF..."
sudo ./upf -c "${ROOT_DIR}/configs/upf.yaml" &
UPF_PID=$!

sleep 0.5
echo "[*] Running SMF demo (PDU session establish/modify/delete)..."
sudo ./smf -c "${ROOT_DIR}/configs/smf.yaml" --demo-pdu

echo "[*] Stopping UPF..."
sudo kill ${UPF_PID} || true
