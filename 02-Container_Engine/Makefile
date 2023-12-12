# Hostname of the container.
CONTAINER_NAME=mycont
# Image of the container.
CONTAINER_IMAGE=debian-stable+python+iproute2
# Memory limit (MB) of the container.
CONTAINER_MEMORY=1024
# CPU limit (CPU percent) of the container.
CONTAINER_CPU=100
# IP address of the container.
CONTAINER_IP=192.168.42.10
# IP subnet mask of the container.
CONTAINER_SUBNET=24
# Fully qualified IP address (with subnet mask) of the container.
CONTAINER_FQIP=$(CONTAINER_IP)/$(CONTAINER_SUBNET)

# Compile target: the container manager.
TARGET=contain

# Base command of container run targets.
RUN_CMD="./$(TARGET)" "$(CONTAINER_NAME)" "$(CONTAINER_IMAGE)" "$(CONTAINER_MEMORY)" "$(CONTAINER_CPU)" "$(CONTAINER_FQIP)" bash -c

# Compilation settings
SRCS:=$(wildcard *.c)
HDRS:=$(wildcard *.h)
OBJS:=$(SRCS:.c=.o)
INCLUDES=-I/usr/include/libnl3

CFLAGS+=-Wall -Wextra -Werror \
		-Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable\
		$(INCLUDES)
LDLIBS+=-lnl-3 -lnl-route-3


.PHONY: all clean cleantags mrproper run-% setup reset


all: $(TARGET)


$(TARGET): $(OBJS)

libcontain.o: %.o: %.c %.h


clean:
	$(RM) $(OBJS) $(TARGET)

mrproper: clean cleantags

cleantags:
	rm -f tags


tags: $(SRCS) $(HDRS)
	ctags *


# Container run targets.
RUN_TARGETS=run-namespaces \
			run-namespace-mount \
			run-cgroups \
			run-overlayfs \
			run-networking \
			run-networking-webserver
$(RUN_TARGETS): $(TARGET) reset

# Run target 1: namespaces.
run-namespaces:
	$(RUN_CMD) 'echo User $$(id -u), PID $$$$ \($$(expr $$(echo /proc/[0-9]* | wc -w) - 4) processes\), \
		hostname $$(hostname), $$(ip link | grep -c "^[0-9]*:") network interfaces'

# Run target 2: mount namespace.
run-namespace-mount:
	sudo $(RUN_CMD) 'echo $$(expr $$(echo /proc/[0-9]* | wc -w) - 4) process\(es\); echo "Listing home directories:"; ls /home'

# Run target 3: cgroups.
run-cgroups:
	sudo $(RUN_CMD) 'echo "cgroups of running process:"; grep $(CONTAINER_NAME) /proc/self/cgroup '

# Run target 4: overlay FS.
run-overlayfs:
	sudo $(RUN_CMD) 'echo "creating \"testfile\":"; touch testfile; ls'
	@echo
	@echo ">>> Compare with the content of the container image folder on the host:"
	@ls $(CONTAINER_IMAGE)

# Run target 5: basic networking.
run-networking:
	sudo $(RUN_CMD) 'echo "Network interfaces and IP addresses:"; ip addr'

# Run target 6: networking with containerized web server.
run-networking-webserver:
	@echo ">>> Open your browser at http://$(CONTAINER_IP):8000"
	sudo $(RUN_CMD) 'python3 -m http.server 8000 --bind $(CONTAINER_IP)'


# Initial setup (run once).
setup:
	@./setup.sh

# Reset working directory (run between container runs).
reset:
	@./reset.sh $(CONTAINER_NAME)

