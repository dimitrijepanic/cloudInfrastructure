/*
 * contain -- Simple container engine
 *
 * v0 "base": base empty version.
 *
 * Run a command on an isolated filesystem, with an isolated network stack,
 * under namespaces and cgroups.
 *
 * Requires privileged rights (specifically, CAP_SYS_ADMIN to mount the
 * container's image and to create network objects).
 * Requires some setup (run `make setup`).
 *
 * Requires libcontain (companion helper library).
 * Requires libnl (netlink library).
 */

#define _GNU_SOURCE

#include "libcontain.h"

#include <netlink/netlink.h>
#include <netlink/route/link/veth.h>
#include <netlink/route/link/bridge.h>
#include <netlink/route/addr.h>

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>

#include <linux/sched.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>


static int condition = 0;
/*
 * Print the command to run in the container, and its arguments.
 *
 * Parameters:
 * * cont_args: the command and its arguments
 * * nb_args: the number of arguments
 */
void print_container_cmd(char* const cont_args[], int nb_args) {
    /*** STUDENT CODE BELOW ***/

    char command[500];
    int ptr = 0;
    for (int i = 0; i < nb_args; i++)
    {
        printf("\n%d \"%s\" ", i, cont_args[i]);
        for (int j = 0; j < (int) strlen(cont_args[i]); j++)
            command[ptr++] = cont_args[i][j];
        command[ptr++] = ' ';
    }
    command[ptr] = '\0';

    printf("\nCEO: %s",command);
    system("echo Hello");
    // Print each argument in cont_args.
    return;

    /*** STUDENT CODE ABOVE ***/
}


// Flags for the syscall clone3.

/*** STUDENT CODE BELOW ***/

// Add the flags for each namespace kind, one by one.
// clone(2)
#define CLONE_NAMESPACE_FLAGS CLONE_NEWUSER | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWNS

/*** STUDENT CODE ABOVE ***/

/*
 * Create a child process that runs inside the container.
 *
 * Clone to a child process that runs under namespaces, and in a cgroup.
 *
 * Parameter:
 * * cont: the container
 *
 * Return:
 * * the PID of the child process
 * * -1 on error (and print an error message)
 */
int clone_to_container(struct container *cont) {
    pid_t cont_pid = -1;
    struct clone_args clone_args = {0};

    /*** STUDENT CODE BELOW ***/
    clone_args.exit_signal = SIGCHLD;
    clone_args.flags = CLONE_NAMESPACE_FLAGS | CLONE_INTO_CGROUP;
    clone_args.cgroup =  cont->cgroup.fd; // TO BE EVALUATED
    size_t size = sizeof(clone_args);
    cont_pid = syscall(SYS_clone3, &clone_args, size);
    if (!cont_pid)
    {
        printf("\nCHILD %d\n", getuid());
        return 0;
        //for(int i = 0; i < 6e9; i++){}
    }
    else
    {
        /*int ret = 3;
        int w = waitpid(cont_pid, &ret, __WALL);
        printf("RET VAL : %d %d %d\n", ret, cont_pid, w);*/
        printf("\nPARENT %d\n", getuid());
        cont->pid = cont_pid;
        printf("SVE KUL\n");
        return cont_pid;
    }
    // Initialize the clone_args structure: flags, exit_signal, cgroup.
    // clone3(2)
    // CLONE_NAMESPACE_FLAGS
    

    // Call the clone3 syscall, store the container process PID in cont->pid
    // both in the parent and in the child.
    // syscall(2), clone3(2), getpid(2)

    /*** STUDENT CODE ABOVE ***/

}

/*
 * Signal the container process to start executing.
 *
 * Parameter:
 * * cont_pid: the PID of the container process
 */
void cont_start(pid_t cont_pid) {
    /*** STUDENT CODE BELOW ***/

    // Signal the container process to start executing (use SIGUSR1).
    // kill(2)
    kill(cont_pid, SIGUSR1);
    return;

    /*** STUDENT CODE ABOVE ***/
}


void killer_handler(int x)
{
    condition = 1;
    printf("\nKILLER CALLED :), pid: %d %d\n", getpid(), getuid());
}
/*
 * Set up the container to wait on a signal from the host.
 */
void set_cont_wait() {
    // Signal action structure to wait for the signal from container manager.
    struct sigaction sigact = {0};

    /*** STUDENT CODE BELOW (q17) ***/

    // Initialize the sigact structure: sa_handler, sa_mask
    // sigemptyset(3)

    sigemptyset(&sigact.sa_mask);
    sigact.sa_handler = killer_handler;

    // Set handler for signal from container manager to start executing.
    // sigaction(2)
    
    sigaction(SIGUSR1, &sigact, NULL);
    
    /*** STUDENT CODE ABOVE (q17) ***/
}


/*
 * Make the container wait for the host signal to start executing.
 */
void cont_wait() {
    /*** STUDENT CODE BELOW (q17) ***/

    // Make the container wait on a condition variable.
    //while(c)
    //printf("\nPRE PAUZIRANJA %d\n", condition);
    while(!condition)
    {
        pause();
    }

    //printf("\nOTPAUZIRAN %d\n", condition);
    return;

    /*** STUDENT CODE ABOVE (q17) ***/
}


/*
 * Finalize the container, from the host.
 *
 * 1. map the root user in the container, to the current user
 * 2. set up the network on the host side
 *
 * Parameter:
 * * cont: the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int finalize_host(const struct container *cont) {
    /*** STUDENT CODE BELOW ***/

    // Map the root user to the user ID in the container.
    // libcontain: cgroup_map_root_user (and implement it!)
    cgroup_map_root_user(cont->pid);
    //printf("\nNOVI %d: \n",getuid());

    /*** STUDENT CODE ABOVE ***/

    /*** STUDENT CODE BELOW ***/

    // Make network from the host for the container.
    // libcontain: contnet_make_host (and implement it!)
    contnet_make_host(cont->pid);
    return 0;

    /*** STUDENT CODE ABOVE ***/

    return 0;
}

/*
 * Finalize the container, from the inside of the container.
 *
 * 1. set the hostname
 * 2. mount the pseudo-filesystems: proc, sys and tmp
 * 3. changing the working and root directories
 * 4. set up the network on the container side
 *
 * Parameter:
 * * cont: the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int finalize_cont(const struct container *cont) {
    /*** STUDENT CODE BELOW ***/

    // Set the hostname of the container.
    // sethostname(2)
    char ho[100];
    gethostname(ho, 30);
    printf("\nHOST: %s\n",ho);
    sethostname(cont->hostname, strlen(cont->hostname));
    gethostname(ho, 30);
    printf("\nHOST: %s\n",ho);
    

    /*** STUDENT CODE ABOVE ***/

    contfs_mount_pseudo_fs(&cont->fs);
    /*** STUDENT CODE BELOW ***/

    // Mount the pseudo FSs required by the containerized process
    // libcontain: contfs_mount_pseudo_fs
    

    // Change current working directory to the FS of the container
    // chdir(2)
    printf("\nPRE FS:\n");
    //system("ls ./debian-stable+python+iproute2/root"); //PERMISSION DENIED
    char newroot[100];
    strcpy(newroot, cont->image);
    //strcat(newroot, "/root");

    //contfs_mount(&cont->fs, cont->image);
    system("pwd");
    
    printf("\nY: %d :Y\n", chdir(cont->fs.root));
   
    system("pwd");
     printf("\nR: %d :R\n", chroot("."));
    //printf("\nR: %d :R\n", chroot(cont->fs.root));
    //printf("\nRET %d\n", chdir("root"));
    printf("\nPOSLE FS:\n");
    system("pwd");
    //return 0;

    // Change root of the container to the current working directory
    // chroot(2)

    system("ls /tmp");

    /*** STUDENT CODE ABOVE ***/

    /*** STUDENT CODE BELOW ***/

    // Make network from inside the container
    // libcontain: contnet_make_cont (and implement it!)
    contnet_make_cont(cont->ip_addr);
    

    /*** STUDENT CODE ABOVE ***/

    return 0;
}

static void host_handle_sigint() {
    printf("container interrupted by keyboard\n");
}


/*
 * Run a command in a container.
 *
 * Main function.
 *
 * Parameters:
 * * cont: the container
 * * cont_args: the command to run in the container, and its arguments
 * * nb_cont_args: the number of elements in cont_args
 *
 * Return:
 * * EXIT_SUCCESS on success
 * * does not otherwise return (exists the program with EXIT_FAILURE)
 */
int contain(struct container cont, char* cont_args[], int nb_cont_args) {
    // Stored in struct container, but required to check the return value of the
    // clone syscall.
    pid_t cont_pid = -1;


    printf("running container \"%s\" from image \"%s\"\n",
            cont.hostname, cont.image);


    printf("container command:");
    print_container_cmd(cont_args, nb_cont_args);


    // Make the filesystem for the container.
    if (contfs_make(&cont.fs, cont.image) < 0)
        errx(EXIT_FAILURE, "failed making filesystem for container %s", cont.hostname);


    // Make the cgroup for the container.
    if (cgroup_make(&cont.cgroup, cont.memory_MB, cont.cpu_perc) < 0) {
        if (contfs_demake(&cont.fs) < 0)
            warn("failed demaking filesystem for container %s", cont.hostname);
        
        errx(EXIT_FAILURE, "failed making cgroup for container %s", cont.hostname);
    }

    // Set a signal handler before cloning, to make it active as soon as the
    // container subprocess runs.
    set_cont_wait();

    // Start the process for the container.
    //system("ls /proc");
    if ((cont_pid = clone_to_container(&cont)) > 0) {
        // In the parent process (container manager).

        // Wait status structure used to wait for the container process.
        int wstatus;
        
        /*char ho[100];
        gethostname(ho, 30);
        printf("\nPARENT HOST: %s\n",ho);*/
        printf("\nPARENT PID: %d\n", getpid());
        // Graciously catch SIGINT.
        struct sigaction sigact;
        memset(&sigact, 0, sizeof(sigact));
        sigact.sa_handler = host_handle_sigint;
        sigemptyset(&sigact.sa_mask);
        sigact.sa_flags = 0;
        sigaction(SIGINT, &sigact, NULL);

        // Finalize the environment of the container on the host side.
        if (finalize_host(&cont) < 0) {
            kill(cont.pid, SIGKILL);
            wait(NULL);

            if (cgroup_demake(&cont.cgroup) < 0)
                warn("failed demaking cgroup for container %s", cont.hostname);
            if (contfs_demake(&cont.fs) < 0)
                warn("failed demaking filesystem for container %s", cont.hostname);

            errx(EXIT_FAILURE, "failed setting up container environment from host");
        }

        cont_start(cont_pid);

        printf("container started as process %d\n\n", cont_pid);

        /*** STUDENT CODE BELOW ***/

        int w = wait(&wstatus);
        printf("RET VAL : %d %d %d\n", wstatus, cont_pid, w);
        // Wait for the container process to terminate.
        // wait(2)


        /*** STUDENT CODE ABOVE ***/

        if (WIFEXITED(wstatus))
            printf("\ncontainer process exited normally with code %d\n", WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("\ncontainer process exited because of signal %s\n", strsignal(WTERMSIG(wstatus)));
    } else if (cont_pid == 0) {
        // In the child process (container).

        cont_wait();

        // Esthetics.
        printf("\nCHILD PID: %d\n", getpid());

        /*for (int i = 0; i < 2; i++)
        {
            sleep(1);
            printf("\nUID :%d\n ",getuid());
        }*/
        //system("ls");
        if (finalize_cont(&cont) < 0)
            errx(EXIT_FAILURE, "failed setting up container environment from container");


        /*** STUDENT CODE BELOW ***/
        
        // Execute the command inside the container, never to return unless it
        // failed executing it (e.g., command not found).
        // execvp(2)
        
        //exit(2); Just for testing wstatus.

        execvp(cont_args[0], cont_args);
        
        
        return EXIT_SUCCESS;
        
        /*** STUDENT CODE ABOVE ***/

        err(EXIT_FAILURE, "failed running container child process for container \"%s\"", cont.hostname);
    } else {
        // Cloning to container failed.

        if (cgroup_demake(&cont.cgroup) < 0)
            warn("failed demakin cgroup for container %s", cont.hostname);
        if (contfs_demake(&cont.fs) < 0)
            warn("failed demaking filesystem for container %s", cont.hostname);

        errx(EXIT_FAILURE,
                "failed starting the containerized child process of container "
                "\"%s\"", cont.hostname);
    }

    // The forked child calls execvp so it never reaches this point.
    // The parent waits for its child, so when this code is reached, the
    // container is terminated.

    if (cgroup_demake(&cont.cgroup) < 0)
        warn("failed demakin cgroup for container %s", cont.hostname);
    if (contfs_demake(&cont.fs) < 0)
        warn("failed demaking filesystem for container %s", cont.hostname);

    printf("container \"%s\" terminated\n", cont.hostname);

    return EXIT_SUCCESS;
}

// Help in parsing arguments: the first argument of the container (i.e., the
// command to run inside) is at this ID in argv.
#define FIRST_CONTAINER_ARG 6

/*
 * Entrypoint of the command line program.
 *
 * Parse the arguments to build a struct container, and treat the container's arguments.
 * Eventually calls contain().
 */
int main(int argc, char* argv[]) {
    int ret;

    char* hostname = argv[1];
    char* image = argv[2];
    unsigned int memory_MB = atoi(argv[3]);
    double cpu_perc = atoi(argv[4]);
    char* ip_addr = argv[5];
    int nb_cont_args = argc - FIRST_CONTAINER_ARG;
    //printf("O: %s %s\n", hostname, image);
    if (argc < FIRST_CONTAINER_ARG) {
        fprintf(stderr, "Usage: %s HOSTNAME IMAGE MEMORY_MB CPU_PERC IP_ADDRESS [ARGS...]\n", argv[0]);
        return -2;
    }
    if (memory_MB <= 0 || cpu_perc <= 0)
        errx(-2, "bad resource limit value for memory or CPU");

    struct container cont = {0};
    cont.hostname = hostname;
    cont.image = image;
    cont.memory_MB = memory_MB;
    cont.cpu_perc = cpu_perc;
    cont.ip_addr = ip_addr;

    // Compute paths of the container's FS.
    contfs_set(&cont.fs, cont.hostname, cont.image);
    // Compute path of the container's cgroup.
    cgroup_set(&cont.cgroup, cont.hostname);

    char* *cont_args = malloc((nb_cont_args + 1) * sizeof(argv[0]));
    // Set the array of arguments for execvp (with a NULL element at the end).
    memcpy(cont_args, &argv[FIRST_CONTAINER_ARG], nb_cont_args * sizeof(argv[0]));
    cont_args[nb_cont_args] = NULL;

    ret = contain(cont, cont_args, nb_cont_args);

    free(cont_args);

    return ret;
}
