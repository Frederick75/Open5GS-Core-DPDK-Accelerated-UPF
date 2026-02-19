# Open5GS Core with DPDK-Accelerated UPF -- Project Explanation

## 1. Project Overview

This project involved enhancing and optimizing the Open5GS 5G Core
Network, focusing on the Session Management Function (SMF) and
integrating a DPDK-accelerated User Plane Function (UPF). The objective
was to achieve carrier-grade performance by reducing latency, increasing
throughput, and ensuring full compliance with 3GPP specifications.

Open5GS provided the control plane implementation, while DPDK enabled
high-performance packet processing by bypassing the Linux kernel
networking stack.

------------------------------------------------------------------------

## 2. 5G Core Architecture

The 5G Core is divided into two major planes:

### Control Plane (CP)

Responsible for signaling and session management.

Key Functions:

-   AMF (Access and Mobility Management Function)
-   SMF (Session Management Function)

The SMF manages PDU sessions, allocates UE IP addresses, and controls
the UPF using PFCP protocol.

------------------------------------------------------------------------

### User Plane (UP)

Responsible for forwarding user traffic.

Key Function:

-   UPF (User Plane Function)

The UPF handles:

-   GTP-U encapsulation and decapsulation
-   Packet routing between UE and Data Network
-   QoS enforcement
-   Tunnel management

------------------------------------------------------------------------

## 3. Interfaces Used

  Interface   Purpose
  ----------- ---------------------------------
  N1          UE to AMF signaling
  N2          AMF to gNB signaling
  N3          gNB to UPF user plane (GTP-U)
  N4          SMF to UPF control plane (PFCP)
  N6          UPF to external Data Network

This project primarily focused on N4 and N3 interfaces.

------------------------------------------------------------------------

## 4. SMF Enhancements

### PDU Session Establishment

A PDU session creates a logical connection between:

UE → 5G Core → Data Network

SMF performs:

-   UE IP address allocation
-   UPF selection
-   PFCP session creation
-   Tunnel configuration
-   QoS configuration

------------------------------------------------------------------------

### PDU Session Modification

Handled dynamically when:

-   QoS parameters change
-   Traffic routing changes
-   Session updates required

SMF updates PFCP rules dynamically without interrupting active sessions.

------------------------------------------------------------------------

## 5. PFCP Implementation over N4 Interface

PFCP is used by SMF to control UPF behavior.

PFCP operations implemented:

-   Session Establishment
-   Session Modification
-   Session Deletion
-   Heartbeat monitoring

PFCP rules configured:

-   PDR (Packet Detection Rule)
-   FAR (Forwarding Action Rule)
-   QER (QoS Enforcement Rule)
-   URR (Usage Reporting Rule)

These rules control packet detection, forwarding, QoS, and reporting.

------------------------------------------------------------------------

## 6. DPDK-Accelerated UPF Optimization

Traditional Linux networking introduces latency due to:

-   Interrupt overhead
-   Kernel-user context switching
-   Packet copying

DPDK solves this using:

-   Kernel bypass
-   Poll-mode drivers
-   Zero-copy packet processing
-   Direct NIC access

------------------------------------------------------------------------

### Packet Processing Flow

NIC → DPDK RX Queue → UPF Worker Core → GTP-U Processing → DPDK TX Queue
→ NIC

------------------------------------------------------------------------

### Multi-Core Optimization

Implemented:

-   RSS-based traffic distribution
-   CPU core pinning
-   NUMA-aware memory allocation
-   Dedicated cores for packet processing

Result:

-   Increased throughput
-   Reduced latency
-   Improved scalability

------------------------------------------------------------------------

## 7. GTP-U Tunnel Management

UPF dynamically creates tunnels using PFCP instructions.

Encapsulation:

IP Packet → Add GTP-U Header → Send to gNB

Decapsulation:

Receive GTP-U → Remove Header → Forward to Data Network

------------------------------------------------------------------------

## 8. CI/CD Integration

Implemented automated pipelines for:

-   Build automation
-   Integration testing
-   Deployment automation
-   Continuous validation

This improved:

-   Software reliability
-   Release cycle efficiency
-   Deployment consistency

------------------------------------------------------------------------

## 9. End-to-End Data Flow

User Plane:

UE → gNB → UPF (DPDK accelerated) → Data Network

Control Plane:

UE → AMF → SMF → UPF (PFCP control)

------------------------------------------------------------------------

## 10. Technologies Used

  Component           Technology
  ------------------- -------------
  5G Core             Open5GS
  Control Plane       SMF
  User Plane          UPF
  Packet Processing   DPDK
  Protocols           PFCP, GTP-U
  Programming         C
  Operating System    Linux
  Deployment          CI/CD

------------------------------------------------------------------------

## 11. Key Achievements

-   Enhanced SMF session management implementation
-   Implemented PFCP-based dynamic tunnel management
-   Integrated and optimized DPDK-based UPF
-   Achieved high throughput and low latency performance
-   Improved system scalability and reliability

------------------------------------------------------------------------

## 12. Real-World Applications

This system supports:

-   Telecom operator 5G networks
-   Private 5G networks
-   Enterprise 5G deployments
-   Research and development environments

Enables connectivity for:

-   Smartphones
-   IoT devices
-   Industrial automation
-   Edge computing

------------------------------------------------------------------------

## 13. Resume Summary Statement

Enhanced Open5GS 5G Core by implementing SMF session management and
optimizing DPDK-accelerated UPF, enabling high-performance
PFCP-controlled GTP-U tunnel processing with improved throughput,
reduced latency, and carrier-grade scalability.

------------------------------------------------------------------------

# 14. Full Architecture Diagram

The following diagram illustrates the Open5GS 5G Core architecture with
DPDK‑accelerated UPF.

![Open5GS DPDK
Architecture](Open5GS-Core-DPDK-Accelerated-UPF-Architecture.svg)

This architecture shows:

-   UE connects to gNB over 5G radio interface
-   SMF controls UPF using PFCP over N4 interface
-   UPF forwards packets via DPDK fast-path
-   DPDK bypasses Linux kernel for high performance
-   Traffic flows between UE and Data Network with low latency

SVG file path:
/mnt/data/Open5GS-Core-DPDK-Accelerated-UPF-Architecture.svg
