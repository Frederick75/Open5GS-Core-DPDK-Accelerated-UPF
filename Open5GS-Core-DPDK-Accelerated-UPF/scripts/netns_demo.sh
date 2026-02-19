#!/usr/bin/env bash
set -euo pipefail

# Lab helper: create minimal namespaces for gNB and UPF.
# This is illustrative; adjust IPs for your environment.

sudo ip netns add ns-gnb || true
sudo ip netns add ns-upf || true

sudo ip link add veth-gnb type veth peer name veth-upf || true
sudo ip link set veth-gnb netns ns-gnb
sudo ip link set veth-upf netns ns-upf

sudo ip netns exec ns-gnb ip addr add 10.10.0.1/24 dev veth-gnb || true
sudo ip netns exec ns-upf ip addr add 10.10.0.2/24 dev veth-upf || true

sudo ip netns exec ns-gnb ip link set lo up
sudo ip netns exec ns-upf ip link set lo up
sudo ip netns exec ns-gnb ip link set veth-gnb up
sudo ip netns exec ns-upf ip link set veth-upf up

echo "Namespaces created: ns-gnb (10.10.0.1) <-> ns-upf (10.10.0.2)"
