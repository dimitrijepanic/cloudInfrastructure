#! /usr/bin/bash

# Uses sudo to run commands with privileges.

IMAGE="debian-stable+python+iproute2"
CGROUP_CTRLRS="+cpu +memory"
ROOT_CGROUP="containers"
HOST_BRIDGE="contbr0"
HOST_BRIDGE_IP="192.168.42.1/24"
VETH_CONT_IFNAME="veth-h"

if [ $# -lt 1 ]; then
    echo "Usage: $0 CONT_NAME"
    echo
    echo "* CONT_NAME: name of the previously created container"
    exit 2
fi >&2

cont_name="$1"

pgrep contain | xargs --no-run-if-empty kill -9
for cont_cgroup in /sys/fs/cgroup/"$ROOT_CGROUP"/*/; do
    if [ -d "$cont_cgroup" ]; then
        sudo rmdir "$cont_cgroup"
    fi
done
if mount | grep -q "$IMAGE"; then
    sudo umount "$IMAGE-$cont_name"/run
fi
sudo rm -fr "$IMAGE-$cont_name"
if ip link show dev "$VETH_CONT_IFNAME" > /dev/null 2>&1; then
    sudo ip link del dev "$VETH_CONT_IFNAME"
fi

