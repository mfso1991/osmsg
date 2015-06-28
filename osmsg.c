

/*
 *	@author : YOU ZHOU
 *	kernel email
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <unistd.h>

#define SENDMSG(message) 	syscall(325, message)
#define GETMSG(message)		syscall(326, message) 

struct message 
{
       uid_t user;		//within user space, it will only hold receivers' IDs
       char msg[140];
};

/**
 *	referencing Linux man page example
 */
uid_t get_uid(char *uid_str)
{
		struct passwd pwd;
		struct passwd *result;
		char *buf;
		size_t bufsize;
		int s;

		bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (bufsize == -1)  	/* Value was indeterminate */        
			bufsize = 16384;	/* Should be more than enough */        

		buf = malloc(bufsize);
		if (!buf) 
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		s = getpwnam_r(uid_str, &pwd, buf, bufsize, &result);
		if (!result) 
		{
			if (!s)
				printf("Not found\n");
			else 
			{
				errno = s;
				perror("getpwnam_r");
			}
			exit(EXIT_FAILURE);
		}
		return pwd.pw_uid;
}

/**
 *	referencing Linux man page example
 */
char* get_uid_str(uid_t uid)
{
		struct passwd pwd;
		struct passwd *result;
		char *buf;
		size_t bufsize;
		int s;

		bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (bufsize == -1)		/* Value was indeterminate */      
			bufsize = 16384;	/* Should be more than enough */ 

		buf = malloc(bufsize);
		if (!buf) 
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		s = getpwuid_r(uid, &pwd, buf, bufsize, &result);
		if (!result) 
		{
			if (!s)
				printf("Not found\n");
			else 
			{
				errno = s;
				perror("getpwnam_r");
			}
			exit(EXIT_FAILURE);
		}
		return pwd.pw_name;
}

int main(int argc, char ** argv)
{
	int return_val = 0;
	struct message *msg = (struct message*)malloc(sizeof(struct message));
	if(!msg)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	if(strncmp(argv[1], "-s", 2) == 0)
	{
		if(argc != 3)
		{
			printf("./osmsg -s username message_to_be_sent\n");
			goto rt;
		}
		msg->user = get_uid(argv[2]);
		strncpy(msg->msg, argv[3], 140);
		if(SENDMSG(msg) == 0)
		{
			printf("message sent\n");
			goto rt;
		}
		else
		{
			printf("error sending message\n");
			return_val = -1;
			goto rt;
		}
	}
	if(strncmp(argv[1], "-r", 2) == 0)
	{
		long signal;
		read_msg:
		switch(signal = GETMSG(msg))
		{
			case -2:	
					printf("never any message\n");
					goto rt;
			case -1:
					printf("no unread message\n");
					goto rt;
			default:
					printf("%s said: %s \n",  get_uid_str(msg->user), msg->msg);
					if(signal)
						goto read_msg;
					goto rt;
		}
	}	
	rt:
	free(msg);
	return return_val;
}