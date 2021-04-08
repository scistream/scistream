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
    const char *local_port;
    const char *remote_host;
    const char *remote_port;
    const char *bind_address;
    const char *client_address;
    size_t buffer_size;
    size_t recv_sz;
    size_t fwd_sz;
};

struct struct_rc {
    int server_socket;
    int client_socket;
    int remote_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct sockaddr_in remote_addr;
    struct hostent *remote_host;
};

typedef struct thread_func_arg_struct {
    int op_socket;
    void *cb;
}thread_func_arg;

#endif

