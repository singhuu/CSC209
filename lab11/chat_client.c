#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef PORT
#define PORT 30000
#endif
#define BUF_SIZE 128

int main(void)
{
    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("client: socket");
        exit(1);
    }

    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) < 1)
    {
        perror("client: inet_pton");
        close(sock_fd);
        exit(1);
    }

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("client: connect");
        close(sock_fd);
        exit(1);
    }

    // Get the user to provide a name.
    char buf[2 * BUF_SIZE + 1]; // 2x to allow for usernames
    printf("Please enter a username: ");
    fflush(stdout);
    int num_read = read(STDIN_FILENO, buf, BUF_SIZE);
    buf[num_read-1] = '\0';
    write(sock_fd, buf, num_read);

    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.

    // Intialiazing File Descriptors to be used in the subsequent While loop
    // Preparing to listen
    // Note: The code has been copy pasted form chat_server.c which is also in a similar
    // environment
    int max_fd = sock_fd;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);
    // Additional FD_SET for Input
    FD_SET(STDIN_FILENO, &all_fds);
    // NOTE: Most of the code is implemented the while loop from chat_server.c
    while (1)
    {
        fd_set listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1)
        {
            perror("Client: Select");
            exit(1);
        }
        // Client Side Implementation. First Read
        if (FD_ISSET(STDIN_FILENO, &listen_fds))
        {
            // First Read,
            int num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if (num_read == 0)
            {
                break;
            }
            buf[num_read] = '\0';
            // NOTE TO SELF (MIGHT BE USEFUL IN A4)
            // You created a seperate if statement for the write statement
            // below that caused input issues, will ask TA later why it occurred, but
            // remember this for now
            /* 
            * We should really send "\r\n" too, so the server can identify partial 
            * reads, but you are not required to handle partial reads in this lab.
            */
            int num_written = write(sock_fd, buf, num_read);
            if (num_written != num_read)
            {
                perror("client: write");
                close(sock_fd);
                exit(1);
            }
        }
        // Similar Implementation in the server side
        if (FD_ISSET(sock_fd, &listen_fds))
        {
            int num_read = read(sock_fd, buf, BUF_SIZE);
            if (num_read == -1)
            {
                perror("Server: Read");
                exit(1);
            }
            if (num_read == 0)
            {
                break;
            }
            buf[num_read] = '\0';
            // Commented the line below out because of the output format
            // printf("Received from server: %s", buf);
            // Edit: Prof said it's okay
            printf("Received from server: %s", buf);
        }
    }

    close(sock_fd);
    return 0;
}

