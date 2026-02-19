# Architecture Notes

This codebase models a minimal carrier-grade control/user-plane split:

- SMF performs PDU session lifecycle and configures UPF using PFCP (N4)
- UPF maintains rule tables (PDR/FAR/QER/URR) and forwards traffic (GTP-U N3 ↔ N6)
- Dataplane supports:
  - socket fast-path (default)
  - DPDK fast-path (optional, compile-time + config enabled)

Key design goals:
- Thread-safe session handling
- Robust PFCP transaction management (retries, timeouts)
- Safe parsing/encoding boundaries
- Extensible rule model

See `Open5GS-Core-DPDK-Accelerated-UPF.md` for project-level explanation.
