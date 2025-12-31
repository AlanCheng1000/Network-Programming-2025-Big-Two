extern "C"{
  #include "../../../lib/unp.h"
  #include  <string.h>
  #include  <stdio.h>
  #include <string.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
}

char id[MAXLINE];

void clear_shell(FILE *fp){
  fputs("\033[H\033[2J", fp);
}

void clr_scr() {
	printf("\x1B[2J");
};

void set_scr() {		// set screen to 80 * 25 color mode
	printf("\x1B[=3h");
};

void big2_cli(FILE *fp, int sockfd)
{
    int       maxfdp1, stdineof, peer_exit, n;
    fd_set    rset;
    char      sendline[MAXLINE], recvline[MAXLINE];

	
    set_scr();
    clr_scr();
    
    Writen(sockfd, id, strlen(id));
    printf("sent: %s\n", id);
    
    // read four players joined msg
    bzero(recvline, MAXLINE);
    Read(sockfd, recvline, MAXLINE);
    printf("recv: %s", recvline);
    
    // read players name msg
    bzero(recvline, MAXLINE);
    Read(sockfd, recvline, MAXLINE);
    printf("recv: %s", recvline);
    
    // read abbreviation msg
    bzero(recvline, MAXLINE);
    Read(sockfd, recvline, MAXLINE);
    printf("recv: %s", recvline);
    
    stdineof = 0;
    peer_exit = 0;

    for ( ; ; ) {	
	FD_ZERO(&rset);
	maxfdp1 = 0;
        if (stdineof == 0) {
            FD_SET(fileno(fp), &rset);
	    maxfdp1 = fileno(fp);
	};
	if (peer_exit == 0) {
	    FD_SET(sockfd, &rset);
	    if (sockfd > maxfdp1){
		maxfdp1 = sockfd;
	    }
	};	
        maxfdp1++;
        Select(maxfdp1, &rset, NULL, NULL, NULL);

	if (FD_ISSET(sockfd, &rset)) {  /* socket is readable */
	    bzero(recvline, MAXLINE);
	    n = read(sockfd, recvline, MAXLINE);
	    if (n == 0) {
       		if (stdineof == 1)
                        return;         /* normal termination */
       		else {
                        printf("(End of input from the peer!)\n");
                        peer_exit = 1;
                };
            }
	    else if (n > 0) {
	        if(recvline[0] == '-'){
	            clear_shell(stdout);
	        }
	        printf("\x1B[0;36m%s\x1B[0m", recvline);
	        fflush(stdout);
	    };
          }
		
        if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
	    char *first_char = Fgets(sendline, MAXLINE, fp);
            if (first_char == NULL || strcasestr(sendline, "bye") != NULL) {
		  if (peer_exit)
			  return;
		  else {
			  printf("(leaving...)\n");
			  stdineof = 1;
			  Shutdown(sockfd, SHUT_WR);      /* send FIN */
		  };
            }
	    else {
		    n = strlen(sendline);
		    Writen(sockfd, sendline, n);				
	    };
        }
    }
};

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 3)
		err_quit("usage: tcpcli <IPaddress> <ID>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT+4);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	strcpy(id, argv[2]);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	big2_cli(stdin, sockfd);		/* do it all */

	exit(0);
}
