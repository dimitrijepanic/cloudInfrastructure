#! /usr/bin/bash

# Uses sudo to run commands with privileges.

IMAGE="debian-stable+python+iproute2"
CGROUP_CTRLRS="+cpu +memory"
ROOT_CGROUP="containers"
HOST_BRIDGE="contbr0"
HOST_BRIDGE_IP="192.168.42.1/24"

die() {
    echo "$*" 1>&2
    exit 1
}

mkdir "$IMAGE" ||
    die "failed creating image directory \"$IMAGE\""
tar -xf "$IMAGE".tar.xz -C"$IMAGE" ||
    die "failed extracting image archive \"$IMAGE.tar.xz\" to \"$IMAGE\""
echo "$CGROUP_CTRLRS" | sudo tee /sys/fs/cgroup/cgroup.subtree_control > /dev/null ||
    die "failed enabling controllers \"$CGROUP_CTRLRS\" in cgroups"
sudo mkdir /sys/fs/cgroup/"$ROOT_CGROUP" ||
    die "failed creating root cgroup \"$ROOT_CGROUP\" for containers"
echo "$CGROUP_CTRLRS" | sudo tee /sys/fs/cgroup/"$ROOT_CGROUP"/cgroup.subtree_control > /dev/null ||
    die "failed enabling controllers \"$CGROUP_CTRLRS\" in root container cgroups\"$ROOT_CGROUP\""
sudo ip link add name "$HOST_BRIDGE" type bridge ||
    die "failed creating host bridge \"$HOST_BRIDGE\" for containers"
sudo ip addr add "$HOST_BRIDGE_IP" dev "$HOST_BRIDGE" ||
    die "failed assigning IP address \"$HOST_BRIDGE_IP\" to host bridge \"$HOST_BRIDGE\" for containers"
sudo ip link set dev "$HOST_BRIDGE" up

