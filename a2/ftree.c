#include <stdio.h>
// Add your system includes here.
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ftree.h"

struct TreeNode *ftree_helper(const char *fname, const char *path)
{
    // First Sub-Part of the function
    // Creating a new tree node and copying the contents to the node
    struct TreeNode *helper_node = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    helper_node->fname = (char *)malloc(sizeof(char) * (strlen(fname) + 1));
    strcpy(helper_node->fname, fname);
    // Concatenating the path and the file name since there is not better faster way to do it than this
    char *new_path = (char *)malloc(sizeof(char) * (strlen(fname) + strlen(path) + 1 + 1));
    strcpy(new_path, path);
    strcat(new_path, fname);
    struct stat var;
    int temp = lstat(new_path, &var);
    if (S_ISREG(var.st_mode)) // Checks if it is a variable
    {
        helper_node->type = '-';
    }
    if (S_ISDIR(var.st_mode)) // Checks if it is a directory
    {
        helper_node->type = 'd';
    }
    if (S_ISLNK(var.st_mode)) // Checks if it is a symbolic link
    {
        helper_node->type = 'l';
    }
    if (temp == -1) // Error statement
    {
        perror("something regarding lstat looks incorrect");
    }

    helper_node->permissions = var.st_mode & 0777; // Extracting permissions (taken from the handout)

    // Second Sub-Part of the function
    if (helper_node->type == 'd') // Checking if it's a directory
    {
        // Opening the Directory. Don't forget to close it.
        DIR *directory;
        directory = opendir(new_path);
        strcat(new_path, "/"); // Adding a / at the end of the path to show that the path has been read and is ready to go into the subdirectory

        // Creating a pointer to the dirent structre
        struct dirent *dir;
        dir = readdir(directory); // function reads the directory

        while (dir != NULL && (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)) // Loop checks if directory is not empty
        {
            dir = readdir(directory);
        }

        // Probably a more efficient way to do this, but oh well..
        if (dir == NULL)
        {
            helper_node->contents = NULL;
        }
        else // Recurse the function here
        {
            helper_node->contents = ftree_helper(dir->d_name, new_path);
        }
        // Creating a temp struct that takes in the contents
        struct TreeNode *a = helper_node->contents;
        dir = readdir(directory);
        while (dir != NULL)
        {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
            {
                a->next = ftree_helper(dir->d_name, new_path); // Recursion is used here again
                a = a->next;
            }
            dir = readdir(directory); // Stores the next pointer
        }
        closedir(directory);
    }
    free(new_path);
    // At the end of the file
    helper_node->next = NULL;
    if (helper_node->type == '-' || helper_node->type == 'l')
    {
        helper_node->contents = NULL;
    }
    return helper_node;
}
/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing dir!\n", fname);
 *
 */
struct TreeNode *generate_ftree(const char *fname)
{
    // Your implementation here.

    // Hint: consider implementing a recursive helper function that
    // takes fname and a path.  For the initial call on the
    // helper function, the path would be "", since fname is the root
    // of the FTree.  For files at other depths, the path would be the
    // file path from the root to that file.

    DIR *d = opendir(fname); // Creating a Directory Stream of fname. BRB initializing dirent, Man pages are actually helpful
    // Checking if d is not Uninitialized.
    if (d == NULL)
    {
        fprintf(stderr, "The path (%s) does not point to an existing dir!", fname); // Copy-pasted this from the handout -> Tasks subsection
        exit(1);
    }
    closedir(d); // Closed it. NO Memory Leaks ever when I code (ﾟヮﾟ)
    struct TreeNode *node;
    node = ftree_helper(fname, ""); // Using a helper function, the function became too long when I first coded it on my pc and not on the teach.cs system.

    return node; // Tried to shove the function declaration and assignment but the compiler doesn't like it
}

/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 *
 * The only print statements that you may use in this function are:
 * printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions)
 * printf("%s (%c%o)\n", root->fname, root->type, root->permissions)
 *
 */
void print_ftree(struct TreeNode *root)
{

    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    printf("%*s", depth * 2, "");

    // Your implementation here.
    if (root != NULL)
    {
        // Quite straightforward this one, if else statement to distinguish a directory
        if (root->type == 'd')
        {
            // Copy-pasting this line of code from the function comment above
            printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
            // Then we check if the contents are not empty.
            // If they aren't then then we dive into it print it's contents and swim back up
            if (root->contents != NULL)
            {
                depth += 1;                  // Diving In
                print_ftree(root->contents); // Printing the contents inside
                depth -= 1;                  // Swimming back up
            }
        }
        else
        {
            // Copy-pasting this line of code from the function comment above
            printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
        }
    }
    if (root != NULL)
    {
        if (root->next != NULL)
        {
            print_ftree(root->next);
        }
    }
}

/* 
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 * 
 */
void deallocate_ftree(struct TreeNode *node)
{
    if (node != NULL)
    {
        if (node->type == 'd')
        {
            // Once we have verified that its' a directory, we First de-allocate the contents of the node
            if (node->contents != NULL)
            {
                deallocate_ftree(node->contents);
            }
        }
        // Next we de-allocate the pointers that the nodes point to
        if (node->next != NULL)
        {
            deallocate_ftree(node->next);
        }
        // TODO: Since we are being marked to prevent memory leaks, Do not forget to free the memory.
        // I will free it here so that my marks don't get cut.
        free(node->fname);
        free(node);
    }
}

