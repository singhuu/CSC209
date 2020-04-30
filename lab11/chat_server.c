#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef PORT
#define PORT 58555
#endif
#define MAX_BACKLOG 5
#define MAX_CONNECTIONS 12
#define BUF_SIZE 128

struct sockname
{
    int sock_fd;
    char *username;
};

/* Accept a connection. Note that a new file descriptor is created for
 * communication with the client. The initial socket descriptor is used
 * to accept connections, but the new socket is used to communicate.
 * Return the new client's file descriptor or -1 on error.
 */
int accept_connection(int fd, struct sockname *usernames)
{
    int user_index = 0;
    while (user_index < MAX_CONNECTIONS && usernames[user_index].sock_fd != -1)
    {
        user_index++;
    }

    if (user_index == MAX_CONNECTIONS)
    {
        fprintf(stderr, "server: max concurrent connections\n");
        return -1;
    }

    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0)
    {
        perror("server: accept");
        close(fd);
        exit(1);
    }

    // Allocating the user name size according to BUF_SIZE
    usernames[user_index].sock_fd = client_fd;
    char *usr = malloc(sizeof(char) * BUF_SIZE);
    read(client_fd, usr, BUF_SIZE);

    usernames[user_index].username = usr;
    return client_fd;
}

/* 
 * Read a message from client_index and echo it back to them.
 * Return the fd if it has been closed or 0 otherwise.
 */
int read_from(int client_index, struct sockname *usernames)
{
    int fd = usernames[client_index].sock_fd;
    char buf[BUF_SIZE];
    /*
     * In Lab 10, you focused on handling partial reads. For this lab, you do
     * not need handle partial reads.  Because of that, this server program
     * does not check for "\r\n" when it reads from the client. 
     */
    // Take the input
    // Note to self: Don't forget there are 2 usernames
    int num_read = read(fd, &buf, BUF_SIZE);
    buf[num_read] = '\0';
    // NOTE: This code (Line 78) below was causing an extra output during my testing so I felt
    // it necessary to comment it out and it wasn't the same as the provided output
    if (num_read == 0) // || write(fd, buf, strlen(buf)) != strlen(buf))
    {
        usernames[client_index].sock_fd = -1;
        // Since we are mallocing we will have to free the username
        free(usernames[client_index].username);
        return fd;
    }
    // Displaying the output according to the sample files provided
    char res[BUF_SIZE];
    strcpy(res, usernames[client_index].username);
    strcat(res, ": ");
    strcat(res, buf);

    // Support for multiple users by creating new structs
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        // Creating a new temporary struct to send the message back, using the write function
        struct sockname *temp = i + usernames;
        write(temp->sock_fd, res, strlen(res));
    }
    return 0;
}

int main(void)
{
    struct sockname usernames[MAX_CONNECTIONS];
    for (int index = 0; index < MAX_CONNECTIONS; index++)
    {
        usernames[index].sock_fd = -1;
        usernames[index].username = NULL;
    }

    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("server: socket");
        exit(1);
    }

    // Set information about the port (and IP) we want to be connected to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // This sets an option on the socket so that its port can be reused right
    // away. Since you are likely to run, stop, edit, compile and rerun your
    // server fairly quickly, this will mean you can reuse the same port.
    int on = 1;
    int status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *)&on, sizeof(on));
    if (status == -1)
    {
        perror("setsockopt -- REUSEADDR");
    }

    // This should always be zero. On some systems, it won't error if you
    // forget, but on others, you'll get mysterious errors. So zero it.
    memset(&server.sin_zero, 0, 8);

    // Bind the selected port to the socket.
    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("server: bind");
        close(sock_fd);
        exit(1);
    }

    // Announce willingness to accept connections on this socket.
    if (listen(sock_fd, MAX_BACKLOG) < 0)
    {
        perror("server: listen");
        close(sock_fd);
        exit(1);
    }

    // The client accept - message accept loop. First, we prepare to listen to multiple
    // file descriptors by initializing a set of file descriptors.
    int max_fd = sock_fd;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);

    while (1)
    {
        // select updates the fd_set it receives, so we always use a copy and retain the original.
        fd_set listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1)
        {
            perror("server: select");
            exit(1);
        }

        // Is it the original socket? Create a new connection ...
        if (FD_ISSET(sock_fd, &listen_fds))
        {
            int client_fd = accept_connection(sock_fd, usernames);
            if (client_fd > max_fd)
            {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);
            printf("Accepted connection\n");
        }

        // Next, check the clients.
        // NOTE: We could do some tricks with nready to terminate this loop early.
        for (int index = 0; index < MAX_CONNECTIONS; index++)
        {
            if (usernames[index].sock_fd > -1 && FD_ISSET(usernames[index].sock_fd, &listen_fds))
            {
                // Note: never reduces max_fd
                int client_closed = read_from(index, usernames);
                if (client_closed > 0)
                {
                    FD_CLR(client_closed, &all_fds);
                    printf("Client %d disconnected\n", client_closed);
                }
                else
                {
                    printf("Echoing message from client %d\n", usernames[index].sock_fd);
                }
            }
        }
    }

    // Should never get here.
    return 1;
}

