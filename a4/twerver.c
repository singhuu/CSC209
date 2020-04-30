#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "socket.h"

#ifndef PORT
#define PORT 58514
#endif

#define LISTEN_SIZE 5
#define WELCOME_MSG "Welcome to CSC209 Twitter! Enter your username: "
#define SEND_MSG "send"
#define SHOW_MSG "show"
#define FOLLOW_MSG "follow"
#define UNFOLLOW_MSG "unfollow"
#define JOIN_MSG "Has Just Joined\n"
#define BUF_SIZE 256
#define MSG_LIMIT 8
#define FOLLOW_LIMIT 5

struct client
{
    int fd;
    struct in_addr ipaddr;
    char username[BUF_SIZE];
    char message[MSG_LIMIT][BUF_SIZE];
    struct client *following[FOLLOW_LIMIT]; // Clients this user is following
    struct client *followers[FOLLOW_LIMIT]; // Clients who follow this user
    char inbuf[BUF_SIZE];                   // Used to hold input from the client
    char *in_ptr;                           // A pointer into inbuf to help with partial reads
    struct client *next;
};

// Provided functions.
void add_client(struct client **clients, int fd, struct in_addr addr);
void remove_client(struct client **clients, int fd);

// These are some of the function prototypes that we used in our solution
// You are not required to write functions that match these prototypes, but
// you may find them helpful when thinking about operations in your program.
// Send the message in s to all clients in active_clients.
void announce(struct client *active_clients, char *s)
{
    // Looping through structs using structs is difficult
    struct client *temp;
    temp = active_clients;
    while (temp != NULL)
    {
        int write_status_announce = write((*temp).fd, s, strlen(s));
        switch (write_status_announce)
        {
        case -1:
            remove_client(&active_clients, (*temp).fd);
            perror("Client announcement failed");
            break;
        }
        temp = (*temp).next;
    }
}

// Created a seperate function to check newline. I noticed I used this particular block
// of code a lot so I figured it was better to refactor it into a seperate function
int check(const char *temp, int size)
{
    size -= 1;
    for (int i = 0; i < size; i++)
    {
        if (temp[i + 1] == '\n')
        {
            return i + 2;
        }
    }
    return -1;
}
// Move client c from new_clients list to active_clients list.
void activate_client(struct client *c, struct client **active_clients_ptr, struct client **new_clients_ptr)
{
    // NOTE: I am reusing the code from the beginning of the remove_client() function
    // Make a newline in the network
    // Since it's in the network we use inbuf and BUF_SIZE

    // We will first search if the client is in the new_clients list and then move it to active
    // clients list

    struct client **p_act_client;
    // Searching through, the following code was taken from a function that was previously defined
    for (p_act_client = new_clients_ptr; *p_act_client && (*p_act_client)->fd != (*c).fd; p_act_client = &(*p_act_client)->next)
        ;
    // Once we have found it out, we move it over to the ACTIVE list from the NEW clients list
    strcat((*c).username, (*c).inbuf);
    int temp_size = check((*c).inbuf, BUF_SIZE);
    (*c).username[temp_size - 2] = '\0';
    if (*p_act_client == NULL)
    {
        perror("Error in removing the client. See activate_client()");
    }
    else
    {
        // Move and Delete
        (*c).next = *active_clients_ptr;
        *active_clients_ptr = c;
        *p_act_client = NULL;
        *p_act_client = (**p_act_client).next;
    }
}

// The set of socket descriptors for select to monitor.
// This is a global variable because we need to remove socket descriptors
// from allset when a write to a socket fails.
fd_set allset;

/* 
 * Create a new client, initialize it, and add it to the head of the linked
 * list.
 */
void add_client(struct client **clients, int fd, struct in_addr addr)
{
    struct client *p = malloc(sizeof(struct client));
    if (!p)
    {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));
    p->fd = fd;
    p->ipaddr = addr;
    p->username[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *clients;

    // initialize messages to empty strings
    for (int i = 0; i < MSG_LIMIT; i++)
    {
        p->message[i][0] = '\0';
    }
    // Need to initialize following
    for (int i = 0; i < FOLLOW_LIMIT; i++)
    {
        (*p).following[i] = NULL;
    }
    // Similarly, a function for followers
    for (int i = 0; i < FOLLOW_LIMIT; i++)
    {
        (*p).followers[i] = NULL;
    }
    // Finishing up with inbuf, in_ptr and next
    (*p).inbuf[0] = '\0';
    (*p).in_ptr = (*p).inbuf;
    (*p).next = *clients;
    *clients = p;
}

/* 
 * Remove client from the linked list and close its socket.
 * Also, remove socket descriptor from allset.
 */
void remove_client(struct client **clients, int fd)
{
    struct client **p;

    for (p = clients; *p && (*p)->fd != fd; p = &(*p)->next)
        ;

    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p)
    {
        // TODO: Remove the client from other clients' following/followers
        // lists
        //
        // PROGRAMMER COMMENT : In order to do this, We use the pointer p to delete
        // him from the following and followers
        for (int i = 0; i < FOLLOW_LIMIT; i++)
        {
            if ((**p).following[i] != NULL) // Checking if the follower exists
            {
                for (int k = 0; k < FOLLOW_LIMIT; k++)
                {
                    if (p[k] != NULL)
                    {
                        if (fd == (*p[k]).fd)
                        {
                            p[k] = NULL;
                            break;
                        }
                    }
                }
                printf("Client Removed from Following");
            }
            if ((**p).followers[i] != NULL)
            {
                // NOTE: This if loop follows a similar structure as the one above
                for (int k = 0; k < FOLLOW_LIMIT; k++)
                {
                    if (p[k] != NULL)
                    {
                        if (fd == (*p[k]).fd)
                        {
                            p[k] = NULL;
                            break;
                        }
                    }
                }
                printf("Followers of Client Removed");
            }
            // Remove the client
            struct client *t = (*p)->next;
            printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
            FD_CLR((*p)->fd, &allset);
            close((*p)->fd);
            free(*p);
            *p = t;
        }
    }
    else
    {
        fprintf(stderr,
                "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}

int main(int argc, char **argv)
{
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    // If the server writes to a socket that has been closed, the SIGPIPE
    // signal is sent and the process is terminated. To prevent the server
    // from terminating, ignore the SIGPIPE signal.
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // A list of active clients (who have already entered their names).
    struct client *active_clients = NULL;

    // A list of clients who have not yet entered their names. This list is
    // kept separate from the list of active clients, because until a client
    // has entered their name, they should not issue commands or
    // or receive announcements.
    struct client *new_clients = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, LISTEN_SIZE);

    // Initialize allset and add listenfd to the set of file descriptors
    // passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1)
    {
        // make a copy of the set before we pass it into select
        rset = allset;

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1)
        {
            perror("select");
            exit(1);
        }
        else if (nready == 0)
        {
            continue;
        }

        // check if a new client is connecting
        if (FD_ISSET(listenfd, &rset))
        {
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd, &q);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd)
            {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_client(&new_clients, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1)
            {
                fprintf(stderr,
                        "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_client(&new_clients, clientfd);
            }
        }

        // Check which other socket descriptors have something ready to read.
        // The reason we iterate over the rset descriptors at the top level and
        // search through the two lists of clients each time is that it is
        // possible that a client will be removed in the middle of one of the
        // operations. This is also why we call break after handling the input.
        // If a client has been removed, the loop variables may no longer be
        // valid.
        int cur_fd, handled;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++)
        {
            if (FD_ISSET(cur_fd, &rset))
            {
                handled = 0;

                // Check if any new clients are entering their names
                for (p = new_clients; p != NULL; p = p->next)
                {
                    // PROGRAMMER NOTE: The TODO is similar to the partial read code in lab11,
                    // which is where I'm going to be taking help from.
                    if (cur_fd == p->fd)
                    {
                        // TODO: handle input from a new client who has not yet
                        // entered an acceptable name
                        int in_dif = (*p).in_ptr - (*p).inbuf;
                        int num_read = read(cur_fd, (*p).in_ptr, BUF_SIZE - in_dif);
                        switch (num_read)
                        {
                        case 0: // We will Remove the (new_)client here by calling remove_client
                            printf("[%d] Read %d bytes\n", cur_fd, num_read);
                            remove_client(&new_clients, cur_fd);
                            break;
                        case -1: // We Will Handle the Error Checking Here
                            perror("Num_Read");
                            exit(1);
                            break;
                        default: // Once the outliers are checked, we activate the client and log it in the server
                            printf("[%d] Read %d bytes\n", cur_fd, num_read);

                            // If we look at the sample server.log provided, we see that we have to check for a new line
                            // If a newline has been found then we activate the client and display the welcome message
                            // across the server
                            // Sounds straightforward but is difficult to implement
                            (*p).in_ptr += num_read;
                            // Me From the Future: Just realized while debugging the statements below would work only when
                            // hand_def_temp is anything except -1
                            int checker = 0;
                            if (check((*p).in_ptr, num_read) != -1)
                            {
                                checker = 1;
                                printf("[%d] Found newline %s\n", cur_fd, p->inbuf);
                            }
                            if (checker == 1)
                            {
                                int lim = 3 * BUF_SIZE + 1;
                                char msg[lim];
                                // Allocate Memory first
                                activate_client(p, &active_clients, &new_clients);
                                int size_diff = (*p).in_ptr - (*p).inbuf;
                                memset((*p).inbuf, '\0', size_diff);
                                (*p).in_ptr = (*p).inbuf; // inbuf is now the new input

                                // Activate Client, Announce arrival and Log to the Server
                                strcpy(msg, (*p).username);
                                strcat(msg, JOIN_MSG);
                                announce(active_clients, msg);
                            }
                        }
                        handled = 1;
                    }
                }
            }

            if (!handled)
            {
                // Check if this socket descriptor is an active client
                for (p = active_clients; p != NULL; p = p->next)
                {
                    if (cur_fd == p->fd)
                    {
                        // TODO: handle input from an active client
                        // This piece of code will be similar to the code above &
                        // will follow a similar structure
                        int in_dif = (*p).in_ptr - (*p).inbuf;
                        int num_read_unhandled = read(cur_fd, (*p).in_ptr, BUF_SIZE - in_dif);
                        switch (num_read_unhandled)
                        {
                        case 0: // We will Remove the (new_)client here by calling remove_client
                            remove_client(&new_clients, cur_fd);
                            printf("[%d] Read %d bytes\n", cur_fd, num_read_unhandled);
                            break;
                        case -1: // We Will Handle the Error Checking Here
                            perror("Num_Read");
                            exit(1);
                            break;
                        default: // Once the outliers are checked, we activate the client and log it in the server
                            printf("[%d] Read %d bytes\n", cur_fd, num_read_unhandled);
                            (*p).in_ptr += num_read_unhandled;
                            if (check((*p).in_ptr, num_read_unhandled) != -1)
                            {
                                // We have to verify if our buffer has the following things:
                                // (1) Has 2 "keywords", mostly in the format <COMMAND> <USERNAME>
                                // (2) Has <SPACE>

                                // Search if the struct has a space, If yes then we break it up into two "keywords"
                                // It is also easier and straightforward to extract the second keyword than the
                                // first one
                                char *key_one = strtok((*p).inbuf, " ");
                                char *key_two = strchr((*p).inbuf, ' ');
                                if (key_two != NULL)
                                {
                                    // The Following will be implemented using valid key_one and key_two:
                                    // (1) SEND_MSG (2) UNFOLLOW_MSG (3) FOLLOW_MSG
                                    // Tried Switch statements here but couldn't make it work
                                    // Send a Message: send <username>
                                    if (strcmp(key_one, SEND_MSG) == 0)
                                    {
                                        // Pretty Straightforward, key_two will be the message that
                                        // will be send to other clients and users
                                        char send_msg_var[BUF_SIZE];
                                        strcpy(send_msg_var, key_two + 1); // + 1 for \0
                                        send_msg_var[check(key_two + 1, BUF_SIZE)] = '\0';

                                        // Send the message now
                                        for (int i = 0; i < MSG_LIMIT; i++)
                                        {
                                            if ((*p).message[i][0] == '\0') // Check if there is an empty slot
                                            {
                                                // Yes, than store it in and exit
                                                strcat((*p).message[i], send_msg_var);
                                                break;
                                            }
                                            switch (i)
                                            {
                                            case MSG_LIMIT - 1:
                                                announce(active_clients, "Your tweet was not sent as you have exceeded your character limit");
                                                break;
                                            }
                                        }
                                    }
                                    else if (strcmp(key_one, FOLLOW_MSG) == 0)
                                    {
                                        // This will follow a similar structure to the if statements above
                                        // We will create the username, check it's validity and they're already following
                                        // and if not then finally follow and display msg
                                        char name_follow[BUF_SIZE];
                                        strcpy(name_follow, key_two + 1);

                                        name_follow[check(key_two + 1, BUF_SIZE) - 1 - 1] = '\0';
                                        // We will have to create a new struct temporarily to check if the username exists
                                        // Using code from above for traversing through the list
                                        for (p = active_clients; p && strcmp(p->username, name_follow) != 0; p = p->next)
                                            ;
                                        struct client *temp_unfollow = p;
                                        // Now we check the conditions ie. (1) Already Followed (2) validity
                                        // If (1) and (2) have failed then we follow
                                        for (int i = 0; i < FOLLOW_LIMIT; i++)
                                        {
                                            if (p->following[i] != NULL)
                                            {
                                                if (strcmp(temp_unfollow->username, p->following[i]->username) == 0)
                                                {
                                                    announce(active_clients, "You have already followed");
                                                }
                                            }
                                        }
                                        if (temp_unfollow == NULL)
                                        {
                                            announce(active_clients, "user does not exist");
                                        }
                                        else
                                        {
                                            // Follow the user requested
                                            // We do this by searching for empty space in both users's list
                                            // It will fail, if one of the two user's have their list full
                                            // Initializing u1 and u2 outside the loop because they are needed after the loops again
                                            int u1 = 0;
                                            int u2 = 0;
                                            for (u1 = 0; u1 < FOLLOW_LIMIT; u1++)
                                            {
                                                if ((*p).following[u1] != NULL)
                                                {
                                                    continue;
                                                }
                                                else
                                                {
                                                    break;
                                                }
                                            }
                                            for (u2 = 0; u2 < FOLLOW_LIMIT; u2++)
                                            {
                                                if ((*temp_unfollow).followers[u2] != NULL)
                                                {
                                                    continue;
                                                }
                                                else
                                                {
                                                    break;
                                                }
                                            }
                                            if (u1 < FOLLOW_LIMIT && u2 < FOLLOW_LIMIT)
                                            {
                                                p->following[u1] = temp_unfollow;
                                                printf("%s is now following %s\n", p->username, temp_unfollow->username);
                                                temp_unfollow->followers[u2] = p;
                                                printf("%s was just followed by %s\n", temp_unfollow->username, p->username);
                                            }
                                            else
                                            {
                                                announce(active_clients, "Follow failed: due to either you or target don't have space");
                                            }
                                        }
                                    }
                                    else if (strcmp(key_one, UNFOLLOW_MSG) == 0)
                                    {
                                        // This will be very similar to the else if statement above
                                        char name_unfollow[BUF_SIZE];
                                        strcpy(name_unfollow, key_two + 1);
                                        name_unfollow[check(key_two + 1, BUF_SIZE) - 1 - 1] = '\0';
                                        // We will have to create a new struct temporarily to check if the username exists
                                        // Using code from above for traversing through the list
                                        for (p = active_clients; p && strcmp(p->username, name_unfollow) != 0; p = p->next)
                                            ;
                                        struct client *temp = p;
                                        for (int i = 0; i < FOLLOW_LIMIT; i++)
                                        {
                                            // Comparing the two usernames
                                            if (strcmp(p->following[i]->username, (*temp).username) == 0)
                                            {
                                                if ((*p).following != NULL)
                                                {
                                                    announce(active_clients, "You have already unfollowed");
                                                }
                                            }
                                        }
                                        if (temp == NULL)
                                        {
                                            announce(active_clients, "user does not exist");
                                        }
                                        else
                                        {
                                            int u1 = 0;
                                            int u2 = 0;
                                            for (u1 = 0; u1 < FOLLOW_LIMIT; u1++)
                                            {
                                                // We will now find them in the struct and delete their entry
                                                // by using the null operator, hence, unfollowing them,
                                                // they will no longer recieve the updates
                                                if (strcmp(p->following[u1]->username, (*temp).username) != 0)
                                                {
                                                    continue;
                                                }
                                                else
                                                {
                                                    break;
                                                }
                                            }
                                            for (u2 = 0; u2 < FOLLOW_LIMIT; u2++)
                                            {
                                                if (strcmp(temp->followers[u1]->username, (*p).username) != 0)
                                                {
                                                    continue;
                                                }
                                                else
                                                {
                                                    break;
                                                }
                                            }
                                            if (u1 < FOLLOW_LIMIT && u2 < FOLLOW_LIMIT)
                                            {
                                                p->following[u1] = NULL;
                                                printf("%s is now unfollowing %s\n", p->username, temp->username);
                                                temp->followers[u2] = NULL;
                                                printf("%s was just unfollowed by %s\n", temp->username, p->username);
                                            }
                                            else
                                            {
                                                announce(active_clients, "UnFollow failed: You had never followed or already unfollowed the previous user");
                                            }
                                        }
                                    }
                                    else
                                    {
                                        announce(active_clients, "Try Again (Command Invalid)\n");
                                    }
                                }
                                else
                                {
                                    // Key_two is empty
                                    // So the following commands will work
                                    // (1) Display Message  (2) Quit
                                    if (strcmp(key_one, SHOW_MSG) == 0)
                                    {
                                        struct client *temp;
                                        char *disp_msg;
                                        // Go through every follower
                                        for (int i = 0; i < FOLLOW_LIMIT; i++)
                                        {
                                            for (int j = 0; j < MSG_LIMIT; j++)
                                            {
                                                temp = p->following[i];
                                                if (temp->message[j][0] != '\0')
                                                {
                                                    // Dynamically allocate space for the message
                                                    // 1 = Semicolon
                                                    // 2 = Space around the semicolon
                                                    // 1 = '\0'
                                                    int size = 1 + 2 + 1 + strlen((*temp).username) + strlen((*temp).message[j]);
                                                    disp_msg = malloc(size * sizeof(char));
                                                    strcat(disp_msg, temp->username);
                                                    strcat(disp_msg, " : ");
                                                    strcat(disp_msg, temp->message[j]);
                                                    announce(active_clients, disp_msg);
                                                }
                                                else
                                                {
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    else if (strcmp(key_one, "Q") == 0)
                                    {
                                        // We just announce and remove the client from the active list
                                        remove_client(&active_clients, (*p).fd);
                                        announce(active_clients, "User disconnected from your network");
                                    }
                                    else
                                    {
                                        announce(active_clients, "Try Again (Command Invalid)\n");
                                    }
                                }
                                // Last but not the least, we have to re-assign ptr and inbuf and refurbish the memset
                                int dif = (*p).in_ptr - (*p).inbuf;
                                memset((*p).inbuf, '\0', dif);
                                // Clear after read
                                (*p).in_ptr = (*p).inbuf;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

