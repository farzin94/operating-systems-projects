#include<stdio.h>

#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<assert.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS 513
#define PATH_MAX 4096
#define HISTORY_DEPTH 100

int count=0;
char history[HISTORY_DEPTH][COMMAND_LENGTH] = {"", ""};

/**
 * Read a command from the keyboard into the buffer "buff" and
 * tokenize it such that tokens[i] points into buff to the i'th
 * token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 * COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into buff.
 * Must be at least NUM_TOKENS long. Will strip out up to one final
 * & token. tokens will be NULL terminated.
 * in_background: pointer to a boolean variable. Set to true if user
 * entered an & as their last token; otherwise set to false.
 */


void add_history(char* tokens[], char* buff){

    int i=count;
    int j=0;
    int k=0;

    // len = sizeof(tokens[0]);
    if (strcmp(tokens[0],"")!=0){
       if (strlen(tokens[0])!=0){


          memset(history[i], 0, strlen(history[i]));
          for(j=0; j<COMMAND_LENGTH; j++){
             if(i>9) i=i-10;
             if (tokens[0][j] == '\0'){
                history[i][j] = ' ';
                k++;
             }
             else
               history[i][j] = tokens[0][j];

            if(tokens[k] == NULL){
               break;
            }
          }
       }
    }

 }

void print_history(){

   int n=1;

   if(count>9){
      n=count-9;
   }
   char string[n];
   char string2[sizeof(history[(n-1)%10])];
   while(n<=count){
      sprintf(string, "%d\t", n);
      write(STDOUT_FILENO, string, strlen(string));
      int j=0;
      //while(history[(n-1)%10][j] != '\0'){
       for(j=0; j<COMMAND_LENGTH; j++){
          string2[j] = history[(n-1)%10][j];
          //j++;
       }
      write (STDOUT_FILENO, string2 ,strlen(string2));
      printf("\n");
      n++;
   }
}

int tokenize_command(char* buff, char* tokens[]){

   int i=0;

   char *local_buff;
   local_buff = strtok(buff, " \t\r\a");

   while(local_buff != NULL){
      tokens[i] = local_buff;
      i++;
      local_buff = strtok(NULL, " \t\r\a");
   }
   tokens[i] = NULL;

   return i;

}



void read_command(char *buff, char *tokens[], _Bool *in_background){

   *in_background = false;

   // Read input
   int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
   if ( (length < 0) && (errno !=EINTR) )
   {
      perror("Unable to read command. Terminating.\n");
      exit(-1); /* terminate with error */
   }

   // Null terminate and strip \n.
   buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

   // Tokenize (saving original command string)
   int token_count = tokenize_command(buff, tokens);
   if (token_count == 0) {
      return;
   }

   // Extract if running in background:
   //assert(strcmp(tokens[token_count - 1], "&") == 0);
   if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
      *in_background = true;
      tokens[token_count - 1] = 0;
   }

}

void handle_SIGINT()
{
   //signal(sig, SIG_IGN);
	write(STDOUT_FILENO, "\n", strlen("\n"));
	print_history();
	//write(STDOUT_FILENO, print_history, strlen(print_history));
	//write(STDOUT_FILENO, "\n", 1);
	//exit(0);
}

/**
 * Main and Execute commands
 */
int main(int argc, char*argv[]){

   char input_buffer[COMMAND_LENGTH];
   char *tokens[NUM_TOKENS];

   //int count=0;
   int result;

   //For Built-in commands
   char *currentDir;
   char *tmpDir;
   char* cwd;
   char dir_holder[PATH_MAX + 1];

		struct sigaction handler;
		handler.sa_handler = handle_SIGINT;
		handler.sa_flags = 0;
		sigemptyset(&handler.sa_mask);
		sigaction(SIGINT, &handler, NULL);

   //signal(SIGINT, handle_SIGINT);

   while (true){
      // Get command
      // Use write becausae we need to use read()/write() to work with
      // signals, and they are incompatible with print()
      currentDir = getcwd(NULL, 0);
      write(STDOUT_FILENO, currentDir, strlen(currentDir));
      write(STDOUT_FILENO, ">", strlen(">"));
      _Bool in_background = false;
      //printf("input_buffer1: %s ", input_buffer);
      input_buffer[0] = '\0';
      read_command(input_buffer, tokens, &in_background);

      // int i=1;
      // strcpy(input_buffer, tokens[0]);
      // while (tokens[i]!=NULL){
      //
      // }
      //    printf("%s", input_buffer);

      if(tokens[0] != NULL){

         if (strcmp(tokens[0], "!!")==0){
            if (history[count-1] < 0){
               write(STDIN_FILENO, "Command not found", strlen("Command not found"));
               write(STDIN_FILENO, "\n", strlen("\n"));
               continue;
            }
            else {
               write(STDOUT_FILENO, history[count-1], strlen(history[count-1]) );
               write(STDIN_FILENO, "\n", strlen("\n"));
               tokenize_command(history[count-1], tokens);
            }
         }

         if (strchr(tokens[0], '!')){
           int line = atoi(&tokens[0][1]);
           if (line <= 0 || line < count-9 || line > count){
             write(STDIN_FILENO, "Command not found", strlen("Command not found"));
             write(STDIN_FILENO, "\n", strlen("\n"));
             continue;
           }
           else{
             write(STDIN_FILENO, history[(line-1)%10], strlen(history[(line-1)%10]) ) ;
            write(STDIN_FILENO, "\n", strlen("\n"));
            tokenize_command(history[(line-1)%10], tokens);
           }
         }
         if (strcmp(tokens[0], "!!")!= 0){
            if(strcmp(tokens[0], "!n")!=0){
               add_history(tokens, input_buffer);
               count++;
            }
        }
   	 	if (strcmp(tokens[0], "exit") == 0){
            return 0;
         }
         if (strcmp(tokens[0], "pwd") == 0){
            cwd = getcwd(dir_holder, (PATH_MAX + 1));
            printf("%s\n", cwd);
            continue;
         }
         if (strcmp(tokens[0], "history") == 0){
            print_history();
            continue;
         }

         // else if (strchr(tokens[0], '!')){
         //   int line = atoi(&tokens[0][1]);
         //   printf("line number %d\n", line);
         //   printf("history %s\n", history[line]);
         //  // read_command(history[line], tokens, &in_background);
         // }
         if (strcmp(tokens[0], "cd") == 0){
            currentDir = (getcwd(NULL, 0));
            if (tokens[1] == NULL)
            {
            	continue;
            }
            if (strchr(tokens[1], '/')){
        			chdir(tokens[1]);
        			printf( "Directory changed to %s\n", tokens[1]);
     		    }
        		else{
        			//currentDir = (getcwd(NULL, 0));
        			tmpDir = strcat(currentDir, "/");
        			tokens[1] = strcat(tmpDir, tokens[1]);
        			chdir(tokens[1]);
        		}
            continue;
 		   }

            pid_t child = fork();

            if (child == 0){
               if (execvp(tokens[0], tokens) < 0) {
                  write(STDOUT_FILENO, "Error: exec failed\n", strlen("Error: exec failed\n"));
                }
               exit(-1);
            }

            else if(child < 0){
               write(STDOUT_FILENO, "Error: fork() failed\n", strlen("Error: fork() failed\n"));
            }
            else {
               if (in_background == false){
                  waitpid(child, &result, 0);
               }
            }
            while (waitpid(-1, NULL, WNOHANG)>0){
               ;
            }

         }

      /**
       * Steps for Basic Shell:
       * 1. Fork a child process
       * 2. Child process invokes execvp() using results in token Array
       * 3. If in_background is false, parent waits for child to finish.
       * Otherwise, parent loops back to read_command() again immediately
       */

   }

   return 0;
}
