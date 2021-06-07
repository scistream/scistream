#ifndef S2DS_H
#define S2DS_H

#define VERSION "0.1"

#define LOCAL_PORT_OPTION     'a'
#define REMOTE_PORT_OPTION    'b'
#define REMOTE_HOST_OPTION    'c'
#define BIND_ADDRESS_OPTION   'd'
#define CLIENT_ADDRESS_OPTION 'e'
#define BUFFER_SIZE_OPTION    'f'
#define FORK_OPTION           'g'
#define LOG_OPTION            'h'
#define STAY_ALIVE_OPTION     'i'
#define HELP_OPTION           'j'
#define VERSION_OPTION        'k'

char *get_current_timestamp(void);

struct struct_settings {
    unsigned int local_port     : 1;
    unsigned int remote_host    : 1;
    unsigned int remote_port    : 1;
    unsigned int bind_address   : 1;
    unsigned int client_address : 1;
    unsigned int buffer_size    : 1;
    unsigned int fork           : 1;
    unsigned int log            : 1;
    unsigned int stay_alive     : 1;
};

struct struct_options {
    const char *local_port;     // Port that S2DS will listen on and the data consumer will connect to
    const char *remote_host;    // IP address of the remote host
    const char *remote_port;    // Port of the remote host
    const char *bind_address;   // IP address S2DS should be binded to
    const char *client_address; // IP address S2DS should accept connection from
    size_t buffer_size;         // Size (in bytes) of receive/forward buffers (Default: 40960)
    size_t recv_sz;             // TODO: Unused
    size_t fwd_sz;              // TODO: Unused
};

struct struct_rc {
    int server_socket; // Socket FD of the server
    int client_socket; // Socket FD of the client
    int remote_socket; // Socket FD of the remote host

    struct sockaddr_in server_addr; // Server address information
    struct sockaddr_in client_addr; // Client address information
    struct sockaddr_in remote_addr; // Remote host address information
    struct hostent *remote_host; // Remote host host information
};

typedef struct thread_func_arg_struct {
    int op_socket; // Socket FD to send/receive data
    void *cb;      // Circular buffer to send/receive data
}thread_func_arg;

#endif
