#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAXPASS 10
#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void)
{
  char user_id[MAXLINE];
  char password[MAXLINE];
  int file_descriptor[2], temp, status;
  /* The user will type in a user name on one line followed by a password 
     on the next.
     DO NOT add any prompts.  The only output of this program will be one 
	 of the messages defined above.
   */

  if (fgets(user_id, MAXLINE, stdin) == NULL)
  {
    perror("fgets");
    exit(1);
  }
  if (fgets(password, MAXLINE, stdin) == NULL)
  {
    perror("fgets");
    exit(1);
  }

  // TODO
  if (pipe(file_descriptor) == -1)
  {
    perror("pipe");
    exit(1);
  }
  temp = fork();
  if (temp < 0)
  {
    perror("fork");
    exit(1);
  }
  else if (temp == 0)
  {
    close(file_descriptor[1]);
    if (dup2(file_descriptor[0], fileno(stdin)) == -1)
    {
      perror("dup2");
    }
    close(file_descriptor[0]);
    execl("./validate", "validate", NULL);
    perror("execl");
    exit(1);
  }
  else
  {
    close(file_descriptor[0]);
    if (write(file_descriptor[1], user_id, MAXPASS) == -1)
    {
      perror("wrong");
      exit(1);
    }
    if (write(file_descriptor[1], password, MAXPASS) == -1)
    {
      perror("wrong");
      exit(1);
    }
    if (wait(&status) == -1)
    {
      perror("wait");
    }
    else
    {
      if (WIFEXITED(status))
      {
        if (WEXITSTATUS(status) == 0)
        {
          printf(SUCCESS);
        }
        else if (WEXITSTATUS(status) == 2)
        {
          printf(INVALID);
        }
        else if (WEXITSTATUS(status) == 3)
        {
          printf(NO_USER);
        }
      }
    }
  }
  return 0;
}
