#include	"unp.h"

void sig_chld(int sig){
        int old_errno = errno;
        pid_t pid;
	while((pid = waitpid(-1, NULL, WNOHANG) > 0)){
	}  // -1: any child; WNOHANG: non-blocking
	errno = old_errno;
}

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid, pidlist[4];  // parent records each child's pid
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	char	sendline[MAXLINE], recvline[MAXLINE];
	int pipefd[2]; /* pipefd[0] reads while pipefd[1] writes*/

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	for (int i = 0; i < 4;) {  // connect to 4 clients (players)
		clilen = sizeof(cliaddr);
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to next iteration of for() */
			else
				err_sys("accept error");
		}
		
                /* accept successfully, increment i */
                i++;
                
                // fork() child dedicated server
                childpid = Fork();
                
		if ( childpid == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			/* process the request */
			if (Readline(connfd, recvline, MAXLINE) == 0){
			        err_quit("tcp serv: client terminated prematurely");
			}
			printf("debug: recieved msg: \"%s\"\n", recvline);
			
			int x;  // access code (password) for all later communications
			char username[32];  // user name
			sscanf(recvline, "%d %s", &x, username);
			
			// clear recvline
			bzero(recvline, MAXLINE);
			
			sprintf(sendline, "%d %s\n", x, s);
			printf("debug: sending msg: \"%s\"\n", sendline);
			
			// send back "<x> <username>\n"
			Writen(connfd, sendline, strlen(sendline));
			
			// wait for 4 players to join
			
			
			// game starts
			for(;;){
			        // recieve client response
			        if (Readline(connfd, recvline, MAXLINE) == 0){
			                err_quit("tcp serv: client terminated prematurely");
			        }
			        
			        // act accordingly
			}
			exit(0);
		}else{
		        // parent stores child  server's pid
		        pidlist[i - 1] = childpid;
		}
		
		/* parent closes connected socket */
		Close(connfd);
	}
	// game logic for parent process
	
}
