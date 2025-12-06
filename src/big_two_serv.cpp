extern "C"{
  #include "../../lib/unp.h"
  #include <stdbool.h>
}

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
	int					listenfd, connfd1[100], connfd2[100];
	pid_t				childpid;
	socklen_t			clilen1, clilen2;
	struct sockaddr_in	cliaddr1, cliaddr2, servaddr;
	char      sendline[MAXLINE], recvline[MAXLINE];
	char      id1[MAXLINE / 2], id2[MAXLINE / 2];
	
	/* to use getpeername() */
	struct sockaddr_in tcp_peeraddr; // use IPv4-specific data structure
	socklen_t addrlen = sizeof(tcp_peeraddr);
	char addr_str[INET_ADDRSTRLEN];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	Signal(SIGCHLD, sig_chld);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT + 4);
	
	/* Set address reuse option */
	int opt = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	    perror("setsockopt(SO_REUSEADDR) failed");
	    close(listenfd);
	    exit(1);
	}

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */
	
	int i = 0;
	for (;;) {
		/* 1st client */
		clilen1 = sizeof(cliaddr1);
		if ( (connfd1[i] = accept(listenfd, (SA *) &cliaddr1, &clilen1)) < 0) {
			if (errno == EINTR)
				continue;		/* back to next iteration of for() */
			else
				err_sys("accept error");
		}
		
		/* recv 1st client's id */
		bzero(recvline, MAXLINE);
		if (Read(connfd1[i], recvline, MAXLINE) == 0){
		        err_quit("tcp serv: client terminated prematurely");
		}
		printf("debug: recieved msg: \"%s\"\n", recvline);
		bzero(id1, MAXLINE / 2);
		sscanf(recvline, "%s", id1);
		
		/* reply to 1st client */
		bzero(sendline, MAXLINE);
		sprintf(sendline, "You are the 1st user. Wait for the second one!\n");
		Writen(connfd1[i], sendline, strlen(sendline));
		
		/* 2nd client */
		clilen2 = sizeof(cliaddr2);
		if ( (connfd2[i] = accept(listenfd, (SA *) &cliaddr2, &clilen2)) < 0) {
			if (errno == EINTR)
				continue;		/* back to next iteration of for() */
			else
				err_sys("accept error");
		}
		
		/* recv 2nd client's id */
		bzero(recvline, MAXLINE);
		if (Read(connfd2[i], recvline, MAXLINE) == 0){
		        err_quit("tcp serv: client terminated prematurely");
		}
		printf("debug: recieved msg: \"%s\"\n", recvline);
		bzero(id2, MAXLINE / 2);
		sscanf(recvline, "%s", id2);
		
		/* tell 1st client about 2nd client*/
		addrlen = sizeof(tcp_peeraddr);
		Getpeername(connfd2[i], (struct sockaddr *) &tcp_peeraddr, &addrlen);  // store tcp localaddr
		Inet_ntop(AF_INET, &(tcp_peeraddr.sin_addr), addr_str, INET_ADDRSTRLEN);  // convert to string
		
		bzero(sendline, MAXLINE);
		sprintf(sendline, "The second user is %s from %s\n", id2, addr_str);
		Writen(connfd1[i], sendline, strlen(sendline));
		
		/* reply to 2nd client */
		bzero(sendline, MAXLINE);
		sprintf(sendline, "You are the 2nd user.\n");
		Writen(connfd2[i], sendline, strlen(sendline));
		
		/* tell 2nd client about 1st client*/
		addrlen = sizeof(tcp_peeraddr);
		Getpeername(connfd1[i], (struct sockaddr *) &tcp_peeraddr, &addrlen);  // store tcp localaddr
		Inet_ntop(AF_INET, &(tcp_peeraddr.sin_addr), addr_str, INET_ADDRSTRLEN);  // convert to string
		
		bzero(sendline, MAXLINE);
		sprintf(sendline, "The first user is %s from %s\n", id1, addr_str);
		Writen(connfd2[i], sendline, strlen(sendline));
		
                /* accept 2 clients successfully, fork() a child server */
		if ( (childpid = Fork()) == 0) {
			int			maxfdp1;
			fd_set		rset;
			bool cli_1_end = false, cli_2_end = false;
			
			Close(listenfd);	/* close listening socket */
			
			FD_ZERO(&rset);
			for ( ; ; ) {
				// Turn on bits for two descriptors
				FD_SET(connfd1[i], &rset);  
				FD_SET(connfd2[i], &rset);
				maxfdp1 = max(connfd1[i], connfd2[i]) + 1; // max descriptor + 1
				Select(maxfdp1, &rset, NULL, NULL, NULL);
				
				/* socket from client 1 is readable */
				if (!cli_1_end && FD_ISSET(connfd1[i], &rset)) {
					bzero(recvline, MAXLINE);
					if (Read(connfd1[i], recvline, MAXLINE) == 0){
						printf("str_cli: cli1 has left\n");
						cli_1_end = true;
					}
					if(!cli_1_end){
						printf("debug: message from client 1: %s\n", recvline);
						
						/* send to client 2 */
						bzero(sendline, MAXLINE);
						sprintf(sendline, "(%s) %s", id1, recvline);
						printf("debug: sending message to client 2: %s\n", sendline);
						Writen(connfd2[i], sendline, strlen(sendline));
					}else if(!cli_2_end){
						/* inform client 2 that client 1 is leaving*/
						bzero(sendline, MAXLINE);
						sprintf(sendline, "(%s is leaving the room)\n", id1);
						Writen(connfd2[i], sendline, strlen(sendline));
					}else{
						/* inform client 2 that client 1 has left*/
						bzero(sendline, MAXLINE);
						sprintf(sendline, "(%s left the room)\n", id1);
						Writen(connfd2[i], sendline, strlen(sendline));
						Shutdown(connfd1[i], SHUT_WR);  /* send FIN to client 1*/
						Read(connfd1[i], recvline, MAXLINE);
						break;
					}
				}
				/* socket from client 2 is readable */
				if (!cli_2_end && FD_ISSET(connfd2[i], &rset)) {
					bzero(recvline, MAXLINE);
					if (Read(connfd2[i], recvline, MAXLINE) == 0){
						printf("str_cli: cli2 has left\n");
						cli_2_end = true;
					}
					if(!cli_2_end){
						printf("debug: message from client 2: %s\n", recvline);
						
						/* send to client 1 */
						bzero(sendline, MAXLINE);
						sprintf(sendline, "(%s) %s", id2, recvline);
						printf("debug: sending message to client 1: %s\n", sendline);
						Writen(connfd1[i], sendline, strlen(sendline));
					}else if(!cli_1_end){
						/* inform client 1 that client 2 is leaving*/
						bzero(sendline, MAXLINE);
						sprintf(sendline, "(%s is leaving the room)\n", id2);
						Writen(connfd1[i], sendline, strlen(sendline));
					}else{
						/* inform client 1 that client 2 has left*/
						bzero(sendline, MAXLINE);
						sprintf(sendline, "(%s left the room)\n", id2);
						Writen(connfd1[i], sendline, strlen(sendline));
						Shutdown(connfd2[i], SHUT_WR);  /* send FIN to client 2*/
						Read(connfd2[i], recvline, MAXLINE);
						break;
					}
				}
			}
			/* both clients have terminated, exit */
			exit(0);
		}
		
		/* parent closes connected socket */
		Close(connfd1[i]);
		Close(connfd2[i]);
		
		/* increment room number */
                i++;
                i = i % 100;  // assume no more than 100 rooms at a time
                
                /* start listening in the next room */
	}
}
