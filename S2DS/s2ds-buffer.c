#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#include "s2ds-buffer.h"
#include "cbuf.h"

struct struct_rc rc;
struct struct_options options;
struct struct_settings settings = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

char rm_host[16]; // Buffer for if remote host is not inputted
char rm_port[6]; // Buffer for if remote port is not inputted

static struct option long_options[] = {
    { "local-port",    required_argument, NULL, LOCAL_PORT_OPTION },
    { "remote-host",   required_argument, NULL, REMOTE_HOST_OPTION },
    { "remote-port",   required_argument, NULL, REMOTE_PORT_OPTION },
    { "bind-address",  required_argument, NULL, BIND_ADDRESS_OPTION },
    { "client-address",required_argument, NULL, CLIENT_ADDRESS_OPTION },
    { "buffer-size",   required_argument, NULL, BUFFER_SIZE_OPTION },
    { "log",           no_argument,       NULL, LOG_OPTION },
    { "stay-alive",    no_argument,       NULL, STAY_ALIVE_OPTION },
    { "help",          no_argument,       NULL, HELP_OPTION },
    { "version",       no_argument,       NULL, VERSION_OPTION },
    { 0, 0, 0, 0 }
};

/* Get current data and time information
 * Arguments:
 *   None
 * Return Value:
 *   char* containing a formatted timestamp
 */
char *get_current_timestamp(void){
    static char date_str[20];
    time_t date;

    time(&date);
    strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", localtime(&date));
    return date_str;
}

/* Populate options struct using passed-in function arguments to main
 * Arguments:
 *   int argc     - argument count
 *   char *argv[] - argument vector
 * Return Value:
 *   None
 */
void set_options(int argc, char *argv[]){
    int opt;
    int index;
    options.buffer_size = 40960; // Default circular buffer size

    do{
        opt = getopt_long(argc, argv, "", long_options, &index);
        switch (opt){
            case LOCAL_PORT_OPTION:{
                options.local_port = optarg;
                settings.local_port = 1;
                break;
            }

            case REMOTE_PORT_OPTION:{
                options.remote_port = optarg;
                settings.remote_port = 1;
                break;
            }

            case REMOTE_HOST_OPTION:{
                options.remote_host = optarg;
                settings.remote_host = 1;
                break;
            }

            case BIND_ADDRESS_OPTION:{
                options.bind_address = optarg;
                settings.bind_address = 1;
                break;
            }

            case BUFFER_SIZE_OPTION:{
                options.buffer_size = atol(optarg);
                settings.buffer_size = 1;
                break;
            }

            case CLIENT_ADDRESS_OPTION:{
                options.client_address = optarg;
                settings.client_address = 1;
                break;
            }

            case FORK_OPTION:{
                settings.fork = 1;
                settings.stay_alive = 1;
                break;
            }

            case LOG_OPTION:{
                settings.log = 1;
                break;
            }

            case STAY_ALIVE_OPTION:{
                settings.stay_alive = 1;
                break;
            }

            default:{
                if(opt != -1){
                    printf("[WARN] there are unrecognized argument(s) %d\n", opt);
                }
            }
        }
    }while (opt != -1);
}

/* Initialize the server and begin listening for client connections
 * Arguments:
 *   None
 * Return Value:
 *   0 on success
 *   1 on error
 */
int build_server(void){
    memset(&rc.server_addr, 0, sizeof(rc.server_addr));

    if (settings.local_port) {
      rc.server_addr.sin_port = htons(atoi(options.local_port));
    } else {
      rc.server_addr.sin_port = htons(0);
    }

    rc.server_addr.sin_family = AF_INET;
    rc.server_addr.sin_addr.s_addr = INADDR_ANY;

    rc.server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (rc.server_socket < 0){
        perror("build_server: socket()");
        return 1;
    }

    int optval = 1;
    if (setsockopt(rc.server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
        perror("build_server: setsockopt(SO_REUSEADDR)");
        return 1;
    }

    if (settings.bind_address){
        rc.server_addr.sin_addr.s_addr = inet_addr(options.bind_address);
    }

    if (bind(rc.server_socket, (struct sockaddr *) &rc.server_addr, sizeof(rc.server_addr)) < 0){
        perror("build_server: bind()");
        return 1;
    }

    if (listen(rc.server_socket, 1) < 0) {
        perror("build_server: listen()");
        return 1;
    }

    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    if (getsockname(rc.server_socket, (struct sockaddr *) &sin, &slen) == -1) {
        perror("getsockname");
    } else {
        if (settings.log) {
            printf("> %s: waiting for connection on port %d\n", get_current_timestamp(), ntohs(sin.sin_port));
        }

        if (!settings.local_port) {
          // Used by s2cs.py to get OS-allocated port
          // TODO: If need to print out server IP, need to wait for connection if no bind_address
          printf("%d\n", ntohs(sin.sin_port));
          fflush(stdout);
        }
    }

    return 0;
}

/* Handle incoming client connections to the server
 * Arguments:
 *   None
 * Return Value:
 *   0 on success
 *   1 on error
 */
int wait_for_clients(void){
    unsigned int client_addr_size;
    client_addr_size = sizeof(struct sockaddr_in);
    rc.client_socket = accept(rc.server_socket, (struct sockaddr *) &rc.client_addr, &client_addr_size);
    if (rc.client_socket < 0){
        if (errno != EINTR){
            perror("wait_for_clients: accept()");
        }
        return 1;
    }

    if (settings.client_address && (strcmp(inet_ntoa(rc.client_addr.sin_addr), options.client_address) != 0)){
        if (settings.log){
            printf("> %s: refused request from %s\n", get_current_timestamp(), inet_ntoa(rc.client_addr.sin_addr));
        }
        close(rc.client_socket);
        return 1;
    }

    if (settings.log){
        printf("> %s: request from %s\n", get_current_timestamp(), inet_ntoa(rc.client_addr.sin_addr));
    }
    return 0;
}

/* Build tunnel between the server and remote host
 * Arguments:
 *   None
 * Return Value:
 *   0 on success
 *   1 on error
 */
int build_tunnel(void){
    rc.remote_host = gethostbyname(options.remote_host);
    if (rc.remote_host == NULL){
        perror("build_tunnel: gethostbyname()");
        return 1;
    }

    memset(&rc.remote_addr, 0, sizeof(rc.remote_addr));

    rc.remote_addr.sin_family = AF_INET;
    rc.remote_addr.sin_port = htons(atoi(options.remote_port));

    memcpy(&rc.remote_addr.sin_addr.s_addr, rc.remote_host->h_addr, rc.remote_host->h_length);

    rc.remote_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (rc.remote_socket < 0){
        perror("build_tunnel: socket()");
        return 1;
    }

    // Connect the remote host's socket to the remote host's address
    if (connect(rc.remote_socket, (struct sockaddr *) &rc.remote_addr, sizeof(rc.remote_addr)) < 0){
        perror("build_tunnel: connect()");
        return 1;
    }else{
        printf("successfully connected to remote\n");
    }

    return 0;
}

/* Function for threads receiving data
 * Arguments:
 *   void *arg_ptr - thread_func_arg* containing socket and circular buffer information
 * Return Value:
 *   0 on exit
 */
void *recv_thread_func(void *arg_ptr){
    thread_func_arg *arg = (thread_func_arg *)arg_ptr;
    circular_buffer *cb  = (circular_buffer *)arg->cb;
    printf("starting receiving thread\n");
    char *buffer = malloc(options.buffer_size);
    long cb_cp;
    int recv_cnt;
    long t = time(0);
    float usage;
    while (1){
        cb_cp = cb_free_cp(cb, 1);
        if(cb_cp > 0){
            recv_cnt = recv(arg->op_socket, buffer, MIN(options.buffer_size, cb_cp), 0);
            if(recv_cnt > 0){
                cb_push_back(cb, buffer, recv_cnt);
            }else if (recv_cnt == -1) {
                printf("socket error. recv returned code: %d\n", recv_cnt);
                break;
            }
        }else{
            usleep(10);
        }
        if (time(0) >= t+1)
	    {
            usage = 100*(cb_cp/options.buffer_size);
	        printf("> %s: recv buffer size: %.2f\n", get_current_timestamp(), usage);
	        t = time(0);
	    }
    }
    return 0;
}

/* Function for threads forwarding data
 * Arguments:
 *   void *arg_ptr - thread_func_arg* containing socket and circular buffer information
 * Return Value:
 *   0 on exit
 */
void *fwd_thread_func(void *arg_ptr){
    thread_func_arg *arg = (thread_func_arg *)arg_ptr;
    circular_buffer *cb  = (circular_buffer *)arg->cb;
    printf("starting forwarding thread, %lu\n", options.buffer_size);
    int fwd_cnt;
    long cb_cnt;
    long cb_cp;
    char *buffer = malloc(options.buffer_size);
    long t = time(0);
    float usage;
    while(1){
        cb_cnt = cb_pop_front(cb, buffer, options.buffer_size);
        if(cb_cnt > 0){
            fwd_cnt = send(arg->op_socket, buffer, cb_cnt, 0);
            if (fwd_cnt < 0){
                perror("fwd_thread_func: send()");
                break;
            }
        }else{
            usleep(10); // Note: ideally should be event-driven to minimize latency overhead, or trade-in CPU
        }
        if (time(0) >= t+1)
	    {
            cb_cp = cb_free_cp(cb, 1);
            usage = 100*(cb_cp/options.buffer_size);
	        printf("> %s: fwd buffer size: %.2f\n", get_current_timestamp(), usage);
	        t = time(0);
	    }
    }
    return 0;
}

/* Main function for setting up communication between the server, client, and remote host
 * Arguments:
 *   int argc     - argument count
 *   char *argv[] - argument vector
 * Return Value:
 *   0 on success
 *   1 on error
 */
int main(int argc, char *argv[])
{
    int ret_code;
    set_options(argc, argv);
    ret_code = build_server();
    if(ret_code != 0){
        printf("Building server Failed, is the port used?\n");
        exit(1);
    }

    // s2cs.py will print out desired remote_host and remote_port (ip:port)
    if (!settings.remote_host || !settings.remote_port) {
      if (scanf("%15[^:]:%5s", rm_host, rm_port) != 2) {
        printf("Failed to read in remote host and remote port\n");
        exit(1);
      } else {
        options.remote_host = rm_host;
        options.remote_port = rm_port;
        if (settings.log){
            printf("> %s: Read in remote host and port: %s:%s\n", get_current_timestamp(), rm_host, rm_port);
        }
      }
    }

    // TODO: HANGS HERE
    ret_code = wait_for_clients();
    if(ret_code != 0){
        printf("Building client connection Failed\n");
        exit(1);
    }

    ret_code = build_tunnel();
    if(ret_code != 0){
        printf("Building tunnel Failed, is the remote server started and listening?\n");
        exit(1);
    }

    circular_buffer cbuf_c2s, cbuf_s2c;
    if (CB_SUCCESS != cb_init(&cbuf_c2s, options.buffer_size)){
        printf("MEM error when init\n");
    }

    if (CB_SUCCESS != cb_init(&cbuf_s2c, options.buffer_size)){
        printf("MEM error when init\n");
    }

    // Client to Server
    pthread_t recv_thread_c2s, fwd_thread_c2s;
    thread_func_arg tdf_arg_recv_c2s, tdf_arg_fwd_c2s;
    tdf_arg_recv_c2s.cb = (void *)&cbuf_c2s;
    tdf_arg_fwd_c2s.cb  = (void *)&cbuf_c2s;

    tdf_arg_recv_c2s.op_socket = rc.client_socket;
    pthread_create( &recv_thread_c2s, NULL, recv_thread_func, (void*) &tdf_arg_recv_c2s);

    tdf_arg_fwd_c2s.op_socket  = rc.remote_socket;
    pthread_create( &fwd_thread_c2s,  NULL, fwd_thread_func,  (void*) &tdf_arg_fwd_c2s);

    // Server to Client
    pthread_t recv_thread_s2c, fwd_thread_s2c;
    thread_func_arg tdf_arg_recv_s2c, tdf_arg_fwd_s2c;
    tdf_arg_recv_s2c.cb = (void *)&cbuf_s2c;
    tdf_arg_fwd_s2c.cb  = (void *)&cbuf_s2c;

    tdf_arg_recv_s2c.op_socket = rc.remote_socket;
    pthread_create( &recv_thread_s2c, NULL, recv_thread_func, (void*) &tdf_arg_recv_s2c);

    tdf_arg_fwd_s2c.op_socket  = rc.client_socket;
    pthread_create( &fwd_thread_s2c,  NULL, fwd_thread_func,  (void*) &tdf_arg_fwd_s2c);

    pthread_join( recv_thread_c2s, NULL);
    pthread_join( fwd_thread_c2s,  NULL);
    pthread_join( recv_thread_s2c, NULL);
    pthread_join( fwd_thread_s2c,  NULL);

    exit(0);
}
