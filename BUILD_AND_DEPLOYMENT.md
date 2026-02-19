# Build and Deployment Instructions — Open5GS Core + DPDK-Accelerated UPF

This guide covers:
- Building SMF + UPF binaries
- Running in **socket fast-path** mode (no DPDK dependency)
- Enabling **DPDK** fast-path (optional)
- Deployment patterns (single host, namespaces, containers)
- Operational checks and troubleshooting

---

## 1. Prerequisites

### OS
- Linux (Ubuntu 22.04+ or similar) recommended
- Kernel with standard networking enabled

### Build tools
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libyaml-dev git
```

> `libyaml-dev` is optional. This project ships a minimal YAML subset parser,
> but libyaml improves compatibility if enabled in future extensions.

### Runtime privileges
- UPF typically needs `CAP_NET_ADMIN` and raw socket/packet access
- Run with `sudo` for initial evaluation, or grant capabilities:
```bash
sudo setcap cap_net_admin,cap_net_raw+ep ./build/bin/upf
```

---

## 2. Build (socket dataplane default)

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

Artifacts:
- `build/bin/smf`
- `build/bin/upf`

---

## 3. Configure

Edit example configs:
- `configs/smf.yaml`
- `configs/upf.yaml`

Key fields:
- **N4 (PFCP)**: `smf.n4_local`, `smf.upf_n4_peer`, `upf.n4_bind`
- **N3 (GTP-U)**: `upf.n3_bind` (default UDP/2152)
- **N6**: `upf.n6_bind` (default raw UDP forwarding demo)

---

## 4. Run (single host, minimal)

### Terminal 1: UPF
```bash
sudo ./bin/upf -c ../configs/upf.yaml
```

### Terminal 2: SMF
```bash
sudo ./bin/smf -c ../configs/smf.yaml
```

Expected logs:
- UPF prints PFCP heartbeat + session create/modify/delete
- SMF prints session lifecycle and PFCP transactions

---

## 5. Functional test (built-in)

A lightweight test trigger is included in SMF:
```bash
sudo ./bin/smf -c ../configs/smf.yaml --demo-pdu
```

This will:
1. Create a PDU session
2. Modify it (add/update a FAR/QER)
3. Delete it

---

## 6. Enable DPDK fast-path (optional)

### 6.1 Install DPDK
On Ubuntu (packaged DPDK):
```bash
sudo apt-get install -y dpdk dpdk-dev
```

Or build from source if you need a specific version.

### 6.2 Hugepages + NIC binding (typical)
```bash
# Reserve hugepages
echo 1024 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# Mount hugetlbfs
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge

# (Optional) Bind NIC to vfio-pci / uio_pci_generic per your environment
```

### 6.3 Build with DPDK
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_DPDK=ON ..
cmake --build . -j
```

### 6.4 Run with DPDK EAL args
Edit `configs/upf.yaml`:
- `upf.dpdk.enabled: true`
- `upf.dpdk.eal_args: "-l 2-5 -n 4 --proc-type=auto"`

Run:
```bash
sudo ./bin/upf -c ../configs/upf.yaml
```

> If DPDK is not installed or pkg-config cannot find `libdpdk`, the build will fail when `USE_DPDK=ON`.

---

## 7. Deployment Patterns

### A) Systemd (recommended for hosts)
Create unit files for `smf.service` and `upf.service` with:
- Restart on failure
- Resource limits
- CPU pinning (UPF)

### B) Linux namespaces (lab-grade isolation)
See `scripts/netns_demo.sh` to create:
- `ns-gnb` (N3 sender)
- `ns-upf` (UPF endpoints)

### C) Containers
You can containerize using the included `Dockerfile` (socket fast-path).
For DPDK in containers, prefer:
- `--privileged` or fine-grained capabilities
- `--device=/dev/vfio/*`
- hugepages mounts

---

## 8. Performance & Tuning Checklist

- CPU pinning (isolate UPF dataplane cores)
- Disable CPU frequency scaling for consistent latency
- NUMA-aware hugepage allocation (if DPDK)
- NIC offloads: validate checksum/GRO settings based on your dataplane design
- Use RSS with enough queues to match worker cores

---

## 9. Troubleshooting

### PFCP port in use
PFCP default UDP/8805:
```bash
sudo lsof -iUDP:8805
```

### GTP-U not received
Check:
- `upf.n3_bind` is correct
- firewall rules / iptables
- UDP/2152 reachable in your network namespace

### DPDK EAL init fails
Common issues:
- hugepages not reserved/mounted
- NIC not bound to vfio/uio driver
- insufficient permissions

---

## 10. CI/CD (quick integration)

Example pipeline steps:
1. `cmake -DCMAKE_BUILD_TYPE=Release ..`
2. `cmake --build . -j`
3. Run smoke test:
   - start `upf` (background)
   - run `smf --demo-pdu`
   - assert exit code 0

Add sanitizer jobs for Debug builds:
- `-DENABLE_SANITIZERS=ON`

