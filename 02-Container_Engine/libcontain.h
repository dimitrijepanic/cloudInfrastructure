/*
 * libcontain -- Companion helper library of contain, a simple container engine
 *
 * Includes:
 * * configuration definitions
 * * structures to represent the container and its subcomponents
 * * generic helper functions
 * * helper functions to manage the filesystem of a container, its network and
 *   its cgroup
 *
 * Requires libnl-route.
 */

#ifndef __LIBCONTAIN_H
#define __LIBCONTAIN_H

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/limits.h>


// Maximum length of options to mount the container filesystem.
#define CONTFS_MOUNT_OPTIONS_SZ 3 * PATH_MAX + 28

// Controller file of UID mappings in procfs.
#define PROC_UID_MAP "uid_map"

// Root path to the cgroup of the container; the container's cgroup is appended
// to this root.
#define CGROUP_ROOT "/sys/fs/cgroup/containers"
// Name of the file of the memory controller that controls the high watermark.
#define CGROUP_CTRL_MEMORY_HIGH "memory.high"
// Name of the file of the CPU controller to control the maximum CPU bandwidth.
#define CGROUP_CTRL_CPU_MAX "cpu.max"

// Name of the host bridge for containers (already set up).
#define HOST_BRIDGE "contbr0"
// Name of the veth end on the host.
#define HOST_VETH_NAME "veth-h"
// Name of the veth end in the container.
#define CONT_VETH_NAME "veth-c"

struct container_fs {
    char base[PATH_MAX];
    char root[PATH_MAX];
    char diff[PATH_MAX];
    char workdir[PATH_MAX];
};

struct container_cgroup {
    char path[PATH_MAX];
    int fd;
};

struct container {
    // Parameters.
    char* hostname;
    char* image;
    unsigned int memory_MB;
    double cpu_perc;
    char* ip_addr;

    // Computed parameters.
    struct container_fs fs;
    struct container_cgroup cgroup;

    // State.
    pid_t pid;
};


// Print error message, errno message, and return error value.
#define raise_err(fmt, ...) \
    do {\
        warn(fmt, ## __VA_ARGS__);\
        return -1;\
    } while (0)
// Print error message, and return error value (don't print errno message).
#define raise_msg(fmt, ...) \
    do {\
        warnx(fmt, ## __VA_ARGS__);\
        return -1;\
    } while (0)



void contfs_set(struct container_fs *fs, const char* hostname,
        const char* image);
int contfs_make(const struct container_fs *cont_fs, const char* image);
int contfs_mount_pseudo_fs(const struct container_fs *cont_fs);
int contfs_mount(const struct container_fs *cont_fs, const char* image);
int contfs_demake(const struct container_fs *cont_fs);

int contfs_mkdirs(const struct container_fs *cont_fs);
int contfs_rmdirs(const struct container_fs *cont_fs);

static inline int contfs_make_mount_options(char* mount_options,
        const char* image, const char* diff_path, const char* workdir_path) {
    return snprintf(mount_options, CONTFS_MOUNT_OPTIONS_SZ,
            "lowerdir=%s,upperdir=%s,workdir=%s",
            image, diff_path, workdir_path);
}


int contnet_make_host(pid_t cont_pid);
int contnet_make_cont(const char* ip_addr);

int contnet_connect_host_bridge(struct nl_sock *nlsk,
        struct rtnl_link *veth_host);


void cgroup_set(struct container_cgroup *cont_cgroup, const char* hostname);
int cgroup_make(struct container_cgroup *cont_cgroup, unsigned int memory_MB,
        double cpu_perc);
int cgroup_map_root_user(pid_t cont_pid);
int cgroup_demake(struct container_cgroup *cont_cgroup);

int cgroup_limit_memory(const struct container_cgroup *cgroup,
        unsigned int memory_MB);
int cgroup_limit_cpu(const struct container_cgroup *cgroup, double cpu_perc);


#define rtnl_link_get_kernel_by_name(nlsk, name, link) \
    rtnl_link_get_kernel(nlsk, 0, name, link)

#endif
