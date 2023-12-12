/*
 * libcontain -- Companion helper library of contain, a simple container engine
 *
 * v0 "base": base empty version.
 *
 * See "libcontain.h".
 */

#define _GNU_SOURCE

#include "libcontain.h"

#include <netlink/netlink.h>
#include <netlink/route/link/veth.h>
#include <netlink/route/link/bridge.h>
#include <netlink/route/addr.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
void contfs_set(struct container_fs *fs, const char* hostname, const char* image) {
    // Build the name of the container's filesystem directory from its hostname
    // and its image's name.
    // Use basename of provided image to make it possible to pass a path to the
    // image directory, for convenience.
    snprintf(fs->base, PATH_MAX, "%s-%s", basename(image), hostname);

    strcpy(fs->root, fs->base);
    strcat(fs->root, "/run");
    strcat(strcpy(fs->diff, fs->base), "/.diff");
    strcat(strcpy(fs->workdir, fs->base), "/.workdir");
}


/* Make the filesystem for the container.
 *
 * 1. create the necessary directories for the mountpoint
 * 2. mount the filesystem
 *
 * Parameters:
 * * cont_fs: the representation of the filesystem of the container
 * * image: the path to the image of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contfs_make(const struct container_fs *cont_fs, const char* image) {
    /*** STUDENT CODE BELOW ***/

    // Create the directories used to mount the container FS.
    // libcontain: contfs_mkdirs
    contfs_mkdirs(cont_fs);

    /*** STUDENT CODE ABOVE ***/

    if (contfs_mount(cont_fs, image) < 0) {
        if (contfs_rmdirs(cont_fs) < 0)
            warn("failed removing container filesystem directories");
        raise_msg("failed mounting container filesystem");
    }

    return 0;
}

int contfs_demake(const struct container_fs *cont_fs) {
    struct stat statbuf;
    // To avoid errors because the student has not yet written the code to mount
    // the container filesystem (as an overlay FS or at all), check that the
    // directory exist.
    if (stat(cont_fs->root, &statbuf) < 0) {
        return 0;
    } else {
        // MNT_DETACH to lazily unmount: for timing reasons, the container's
        // filesystem may still be used by the terminating container process.
        if (umount2(cont_fs->root, MNT_DETACH) < 0)
            raise_err("failed unmounting container filesystem at \"%s\"", cont_fs->root);
    }

    return 0;
}

/*
 * Mount the filesystem of the container.
 *
 * Parameters:
 * * cont_fs: the representation of the filesystem of the container
 * * image: the path to the image of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contfs_mount(const struct container_fs *cont_fs, const char* image) {
    char mount_options[CONTFS_MOUNT_OPTIONS_SZ];
    // Prepare the mount options in mount_options for the overlay FS.
    contfs_make_mount_options(mount_options, image, cont_fs->diff, cont_fs->workdir);

    printf("\nCONTAINER IMAGE: %s\n", image);
    /*** STUDENT CODE BELOW ***/

    // Mount the container FS.
    // mount(2)
    printf("\n NOVI FS \n");
    char news[100];
    strcpy(news, "tree ");
    strcat(news, cont_fs->base);
    system(news);
    mount("image", cont_fs->root, "overlay", 0, mount_options);
    system("ls cont_fs->root");

    /*** STUDENT CODE ABOVE ***/

    return 0;
}

int contfs_mkdirs(const struct container_fs *cont_fs) {
    int the_errno;
    printf("\nFS: \n %s \n %s \n %s \n %s", cont_fs->base, cont_fs->root, cont_fs->workdir, cont_fs->diff);
    if (mkdir(cont_fs->base, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST)
            raise_err("failed creating base directory \"%s\"", cont_fs->base);
    }
    if (mkdir(cont_fs->root, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST) {
            the_errno = errno;
            if (rmdir(cont_fs->base) < 0)
                warn("failed removing base directory \"%s\"", cont_fs->base);
            errno = the_errno;
            raise_err("failed creating root directory");
        }
    }
    if (mkdir(cont_fs->workdir, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST) {
            the_errno = errno;
            if (rmdir(cont_fs->root) < 0)
                warn("failed removing root directory \"%s\"", cont_fs->root);
            if (rmdir(cont_fs->base) < 0)
                warn("failed removing base directory \"%s\"", cont_fs->base);
            errno = the_errno;
            raise_err("failed creating workdir directory");
        }
    }
    if (mkdir(cont_fs->diff, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST) {
            the_errno = errno;
            if (rmdir(cont_fs->workdir) < 0)
                warn("failed removing workdir directory \"%s\"", cont_fs->workdir);
            if (rmdir(cont_fs->root) < 0)
                warn("failed removing root directory \"%s\"", cont_fs->root);
            if (rmdir(cont_fs->base) < 0)
                warn("failed removing base directory \"%s\"", cont_fs->base);
            errno = the_errno;
            raise_err("failed creating diff directory");
        }
    }

    return 0;
}

int contfs_rmdirs(const struct container_fs *cont_fs) {
    if (rmdir(cont_fs->root) < 0)
        raise_err("failed removing root directory \"%s\"", cont_fs->root);
    if (rmdir(cont_fs->workdir) < 0)
        raise_err("failed removing workdir directory \"%s\"", cont_fs->workdir);
    if (rmdir(cont_fs->diff) < 0)
        raise_err("failed removing diff directory \"%s\"", cont_fs->diff);
    if (rmdir(cont_fs->base) < 0)
        raise_err("failed removing base directory \"%s\"", cont_fs->base);

    return 0;
}

int contfs_mount_pseudo_fs(const struct container_fs *cont_fs) {
    // This procedure should run "inside the container", i.e., after mountpoints
    // are unshared.

    char pathbuf[PATH_MAX], *pathbuf_end;
    // Save some place to copy last component (longest: "/proc").
    strncpy(pathbuf, cont_fs->root, PATH_MAX-5);
    pathbuf_end = pathbuf + strlen(pathbuf);

    // proc, sys, and tmp mountpoints are supposed to already exist in the
    // container image.
    memcpy(pathbuf_end, "/proc\0", 6);
    printf("\nMOUNTING POINT %s\n", pathbuf);
    if (mount("proc", pathbuf, "proc", 0, NULL) < 0)
        raise_err("failed mounting proc pseudo-filesystem at \"%s\"", pathbuf);
    memcpy(pathbuf_end, "/sys\0", 5);
    if (mount(NULL, pathbuf, "sysfs", 0, NULL) < 0)
        raise_err("failed mounting sysfs pseudo-filesystem at \"%s\"", pathbuf);
    memcpy(pathbuf_end, "/tmp\0", 5);
    if (mount(NULL, pathbuf, "tmpfs", 0, NULL) < 0)
        raise_err("failed mounting tmpfs pseudo-filesystem at \"%s\"", pathbuf);

    return 0;
}

/*
 * Map the root user in the container, to the user running the program.
 *
 * Parameter:
 * * cont_pid: the PID of the process of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */

static void
       update_map(char *mapping, char *map_file)
       {
           int fd;
           size_t map_len;     /* Length of 'mapping' */

           /* Replace commas in mapping string with newlines. */

           map_len = strlen(mapping);
           for (int j = 0; j < (int)map_len; j++)
               if (mapping[j] == ',')
                   mapping[j] = '\n';

           fd = open(map_file, O_RDWR);
           if (fd == -1) {
               fprintf(stderr, "ERROR: open %s: %s\n", map_file,
                       strerror(errno));
               exit(EXIT_FAILURE);
           }

           if (write(fd, mapping, map_len) != (int)map_len) {
               fprintf(stderr, "ERROR: write %s: %s\n", map_file,
                       strerror(errno));
               exit(EXIT_FAILURE);
           }

           close(fd);
       }

int cgroup_map_root_user(pid_t cont_pid) {
    // Path to the UID map file.
    char uidmap_path[PATH_MAX];
    snprintf(uidmap_path, PATH_MAX, "/proc/%d/" PROC_UID_MAP, cont_pid);

    printf("OVDE: %s", uidmap_path);
    // Stream to the UID map file.
    FILE *uid_map_file;
    // User ID to map to root in the container.
    uid_t uid = getuid();

    /*** STUDENT CODE BELOW ***/

    // Map the user ID to root in the container by writing the mapping to the
    // UID map file of the container process.
    // user_namespaces(7)
    // fopen(3), fprintf(3), flose(3)

    //uid_map_file = fopen(uidmap_path, "r");
    char map_buf[100];
    //printf("\nJANA HOCE ISPIS: %d\n", getuid());
    snprintf(map_buf, 100, "0 %jd 1", (intmax_t) getuid()); //Mi ne znamo sta radi ovaj stvarni argument getuid()...
    
    update_map(map_buf, uidmap_path);
    //fclose(uid_map_file);
    return 0;

    /*** STUDENT CODE ABOVE ***/

    return 0;
}

void cgroup_set(struct container_cgroup *cgroup, const char* hostname) {
    snprintf(cgroup->path, PATH_MAX, CGROUP_ROOT "/%s", hostname);
}


/*
 * Set up the cgroup of the container.
 *
 * Create the container's cgroup, and set its memory and CPU limits.
 *
 * Parameters:
 * * cgroup: the representation of the cgroup of the container
 * * memory_MB: the limit of memory usage of the container (in MB)
 * * cpu_perc: the limit of CPU usage of the container (as a percentage of CPU)
 *
 * Return:
 * * a file descriptor of the cgroup (i.e., its directory in the cgroup FS)
 * * -1 on error (and print an error message)
 */
int cgroup_make(struct container_cgroup *cgroup, unsigned int memory_MB, double cpu_perc) {
    /*** STUDENT CODE BELOW ***/

    // Create the cgroup by making its directory at the cgroup path.
    // mkdir(2), inode(7): file modes

    printf("\nMKDIR TEST%d \n", mkdir(cgroup->path, S_IWUSR | S_IROTH | S_IWUSR | S_IRUSR | S_IROTH));

    // Open (take a FD) to the cgroup's directory in cgroup->fd
    // open(2)
    cgroup->fd = open(cgroup->path, O_DIRECTORY | O_PATH, S_IWUSR | S_IRUSR | S_IROTH);
    
    /*** STUDENT CODE ABOVE ***/

    if (cgroup_limit_memory(cgroup, memory_MB) < 0)
        raise_msg("failed setting memory limit");

    if (cgroup_limit_cpu(cgroup, cpu_perc) < 0)
        raise_msg("failed setting CPU limit");

    printf("\nFILE DESC TEST %d %s\n", cgroup->fd, cgroup->path);
    return cgroup->fd;
}

int cgroup_demake(struct container_cgroup *cgroup) {
    if (close(cgroup->fd) < 0)
        raise_err("failed closing cgroup FD");
    cgroup->fd = -1;

    if (rmdir(cgroup->path) < 0)
        raise_err("failed removing cgroup");

    return 0;
}

/*
 * Set the memory limit of the cgroup of the container.
 *
 * Parameters:
 * * cgroup: the representation of the cgroup of the container
 * * memory_MB: the limit of memory usage of the container (in MB)
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int cgroup_limit_memory(const struct container_cgroup *cgroup, unsigned int memory_MB) {
    // FD to a control file.
    int ctrl_fd;
    // Stream to a control file.
    FILE *ctrl_file;

    memory_MB = memory_MB * 1024 * 1024;

    /*** STUDENT CODE BELOW ***/

    // Set the container's cgroup memory limit.
    // 1. take a FD to the control file for the high memory watermark;
    // 2. make a stream (FILE*) from the FD;
    // 3. write the memory limit of the container to the control file;
    // 4. close the stream (it will also close the FD).
    // openat(2), fdopen(3), fprintf(3), fclose(3)
    // CGROUP_CTRL_MEMORY_HIGH
    // cgroup doc, memory controller: https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#memory
    
    int fd = openat(cgroup->fd, CGROUP_CTRL_MEMORY_HIGH, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR | S_IROTH);
    
    //printf("\nFD : %d %d: FD\n", fd, cgroup->fd);
    FILE* stream = fdopen(fd, "w");
    fprintf(stream, "%u", memory_MB);
    fclose(stream);
    /*** STUDENT CODE ABOVE ***/

    return 0;
}

/*
 * Set the CPU limit of the cgroup of the container.
 *
 * Parameters:
 * * cgroup: the representation of the cgroup of the container
 * * cpu_perc: the limit of CPU usage of the container (as a percentage of CPU)
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int cgroup_limit_cpu(const struct container_cgroup *cgroup, double cpu_perc) {
    // FD to a control file.
    int ctrl_fd;
    // Stream to a control file.
    FILE *ctrl_file;
    // Period of CPU scheduling.
    unsigned int cpu_period = 0;
    // Calculated CPU limit from the CPU percentage and the CPU period.
    unsigned int cpu_limit;

    /*** STUDENT CODE BELOW ***/

    // Set the container's cgroup CPU limit.
    // 1. take a FD to the control file for the max CPU bandwidth;
    // 2. make a stream (FILE*) from the FD;
    // 3. read the scheduler period from the control file;
    // 4. write the CPU limit (computed from the CPU percentage limit and the
    //    scheduler period) and the scheduler period of the container to the
    //    control file;
    // 5. close the stream (it will also close the FD).
    // openat(2), fdopen(3), fscanf(3), fprintf(3), fclose(3)
    // CGROUP_CTRL_CPU_MAX
    // cgroup doc, cpu controller: https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#cpu

    int fd = openat(cgroup->fd, CGROUP_CTRL_CPU_MAX, O_RDWR);
    
    FILE* stream = fdopen(fd, "w+");
    //void* p = malloc(sizeof(int));
    char p[4];
    char s2[20];
    fscanf(stream, "%s %s", p, s2);
    cpu_period = atoi(s2);

    printf("\nFDD : %d %d %s %lf %d: FD\n", fd, cgroup->fd, p, cpu_perc, cpu_period);
    fseek(stream, 0 , SEEK_SET);
    fprintf(stream,"%lf %d\n", cpu_period * (cpu_perc / 100), cpu_period);
    
    fclose(stream);
    system("cat /sys/fs/cgroup/containers/mycont/cpu.max");
    return 0;

    /*** STUDENT CODE ABOVE ***/

    return 0;
}

/*
 * Make the network of the container on the host side.
 *
 * 1. create a veth pair
 * 2. put one end of the pair in the container (in its network namespace)
 * 3. set up the network on the host side by plugging the host end of the veth
 *    pair in the host bridge
 *
 * Parameters:
 * * cont_pid: the PID of the process of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contnet_make_host(pid_t cont_pid) {
    int err = 0;
    errno = 0;
    printf("\nPROVERA HOST\n");
    // Netlink socket.
    struct nl_sock *nlsk = nl_socket_alloc();
    if (nlsk == NULL)
        raise_err("failed opening Netlink socket");
    if ((err = nl_connect(nlsk, NETLINK_ROUTE)) < 0)
        raise_err("failed connecting socket: %s", nl_geterror(err));

    // veth endpoints: veth_host on the host, veth_cont in the container.
    struct rtnl_link *veth_cont, *veth_host = rtnl_link_veth_alloc();
    if (veth_host == NULL)
        raise_err("failed allocating veth link");
    veth_cont = rtnl_link_veth_get_peer(veth_host);

    /*** STUDENT CODE BELOW ***/

    // Set the names of the veth endpoints.
    // libnl: rtnl_link_set_name
    // HOST_VETH_NAME, CONT_VETH_NAME
    rtnl_link_set_name(veth_host, HOST_VETH_NAME);
    rtnl_link_set_name(veth_cont, CONT_VETH_NAME);

    // Move veth_cont to the network namespace of the container.
    // libnl: rtnl_link_set_ns_pid
    rtnl_link_set_ns_pid(veth_cont, cont_pid);


    /*** STUDENT CODE ABOVE ***/

    if (contnet_connect_host_bridge(nlsk, veth_host) < 0)
        raise_err("failed connecting host end to host bridge");

    /*** STUDENT CODE BELOW ***/

    // Create veth_host (its peer veth_cont is automatically created).
    // libnl: rtnl_link_add, flags: NLM_F_CREATE
    rtnl_link_add(nlsk, veth_host, NLM_F_CREATE);
    return 0;

    /*** STUDENT CODE ABOVE ***/

    rtnl_link_veth_release(veth_host);

    nl_socket_free(nlsk);

    return 0;
}

int contnet_connect_host_bridge(struct nl_sock *nlsk, struct rtnl_link *veth_host) {
    int err = 0;

    // Bridge master of veth_host.
    struct rtnl_link *host_br;
    if ((err = rtnl_link_get_kernel_by_name(nlsk, HOST_BRIDGE, &host_br)) < 0)
        raise_err("failed looking up host bridge: %s", nl_geterror(err));

    /*** STUDENT CODE BELOW ***/

    // Set veth_host up.
    // libnl: rtnl_link_set_flags
    // man: netdevice(7)
    rtnl_link_set_flags(veth_host, IFF_UP);

    // Put veth_host in its bridge master.
    // libnl: rtnl_link_set_master, rtnl_link_get_ifindex
    rtnl_link_set_master(veth_host, rtnl_link_get_ifindex(host_br));

    /*** STUDENT CODE ABOVE ***/

    rtnl_link_put(host_br);

    return 0;
}


/*
 * Make the network of the container on the container side.
 *
 * 1. set the IP address of the container end of the veth pair
 * 2. set up the container end of the veth pair
 *
 * Parameters:
 * * ip_addr: the IP address of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contnet_make_cont(const char* ip_addr) {
    
    int err = 0;
    errno = 0;

    // Netlink socket.
    struct nl_sock *nlsk = nl_socket_alloc();
    if (nlsk == NULL)
        raise_msg("failed opening Netlink socket");
    if ((err = nl_connect(nlsk, NETLINK_ROUTE)) < 0)
        raise_msg("failed connecting socket: %s", nl_geterror(err));
    printf("\nPROVERAA\n");
    // veth endpoint in the container.
    // veth_cont is the one created by the container manager, queried from the
    // kernel; veth_cont_up is modified with new flags to apply.
    struct rtnl_link *veth_cont, *veth_cont_up = rtnl_link_veth_alloc();
    if ((err = rtnl_link_get_kernel_by_name(nlsk, CONT_VETH_NAME, &veth_cont)) < 0)
        raise_msg("failed getting container veth interface %s", nl_geterror(err));
    
    // IP address of the veth endpoint in the container (parsed from ip_addr).
    struct nl_addr *nladdr_cont;
    nl_addr_parse(ip_addr, AF_INET, &nladdr_cont);
    // IP address of the veth endpoint in the container (for the link).
    struct rtnl_addr *rtnladdr_cont = rtnl_addr_alloc();
    if (rtnladdr_cont == NULL)
        raise_msg("failed allocating veth address");
    /*** STUDENT CODE BELOW ***/

    // Set the address of rtnladdr_cont to be nladdr_cont.
    // libnl: rtnl_addr_add
    rtnl_addr_set_local(rtnladdr_cont , nladdr_cont );
   
    // Set the interface index of rtnladdr_cont to be the index of veth_cont.
    // libnl: rtnl_addr_set_ifindex, rtnl_link_get_ifindex
    rtnl_addr_set_ifindex(rtnladdr_cont, rtnl_link_get_ifindex(veth_cont));
    

    // Add the link address rtnladdr_cont
    // libnl: rtnl_addr_add, no flag
    rtnl_addr_add(nlsk, rtnladdr_cont, 0);

    // Set veth_cont_up up.
    // libnl: rtnl_link_set_flags
    // man: netdevice(7)
    rtnl_link_set_flags(veth_cont_up, IFF_UP);
    

    // Apply changes made in veth_cont_up to veth_cont
    // libnl: rtnl_link_change, no flag
    rtnl_link_change(nlsk, veth_cont, veth_cont_up, 0);

    /*** STUDENT CODE ABOVE ***/

    nl_addr_put(nladdr_cont);
    rtnl_addr_put(rtnladdr_cont);

    rtnl_link_veth_release(veth_cont);

    nl_socket_free(nlsk);
    
    return 0;
}
