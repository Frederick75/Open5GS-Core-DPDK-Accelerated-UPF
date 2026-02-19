# Open5GS Core + DPDK-Accelerated UPF (Production-Level Reference Implementation)

This repository is a **standalone, runnable reference project** inspired by Open5GS-style 5GC session management and a
DPDK-accelerated UPF data path. It is designed to be:

- **Compilable out of the box** (socket fast-path by default)
- **DPDK-capable** when DPDK headers/libs are installed (`-DUSE_DPDK=ON`)
- Structured for **carrier-grade engineering practices**: clear module boundaries, config, logging, CI hooks, and safe defaults.

> Note: This is not the official Open5GS codebase. It is a production-grade educational/reference implementation aligned
> with the same functional ideas (SMF↔UPF control via PFCP, UPF user plane via GTP-U/N3, DN via N6).

## Repo layout

- `src/common` — logging, utils, config
- `src/pfcp` — PFCP message encode/decode + session rule model
- `src/smf` — SMF core: UE-initiated PDU session establish/modify; PFCP client on N4
- `src/upf` — UPF core: PFCP server, rule tables, GTP-U (N3) + N6 forwarding; optional DPDK dataplane
- `configs/` — example configs
- `scripts/` — helper scripts (run, namespaces, perf hints)
- `docs/` — architecture and operations docs

## Quick start (socket dataplane)

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
sudo ./bin/upf -c ../configs/upf.yaml
sudo ./bin/smf -c ../configs/smf.yaml
```

See **Build & Deployment**: `docs/BUILD_AND_DEPLOYMENT.md`.
