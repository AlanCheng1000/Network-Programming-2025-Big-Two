#include	"unp.h"

void clear_shell(FILE *fp){
  fputs("\033[H\033[J", fp);
}

void
big_two_cli(FILE *fp, int sockfd) /* fp here is stdin*/
{
	char	sendline[MAXLINE], recvline[MAXLINE], username[32], check_username[32];
	int x, check_x;  // password
	
	bezero(sendline, MAXLINE);
	sprintf(sendline, "Player, welcome to Big 2.\nPlease enter your name (no space): ");
	write(sendline, strlen(str), 1, fp);
	
	/*Send initial student ID*/
	bezero(recvline, MAXLINE);
	if (Fgets(recvline, MAXLINE, fp) != NULL) {  // read a line of user input
	        sscanf(recvline, "%s", &username);  // store username
	        x = rand() % 10000;  // enerate random password
	        
	        bezero(sendline, MAXLINE);
	        sprintf(sendline, "%d %s", x, username);
		printf("debug: sending: %s\n", sendline);
		Writen(sockfd, sendline, strlen(sendline));  // send initial msg
	}

	/* rcv reply message from server */
	if (Readline(sockfd, recvline, MAXLINE) == 0)
		err_quit("str_cli: server terminated prematurely");
	
	printf("debug: message recieved: %s\n", recvline);
	
	bzero(check_username, 32);
	sscanf(recvline, "%d %s", &check_x, &check_username);
		
	if(x != check_x || strcmp(username, check_username) != 0){
	  fprintf(stderr, "Illegal server reply");
	  exit(-1);
	}	
	exit(0);
}

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	big_two_cli(stdin, sockfd);		/* do it all */

	exit(0);
}
