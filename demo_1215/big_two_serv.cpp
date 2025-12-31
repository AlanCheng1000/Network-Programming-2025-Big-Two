#include "game.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <iterator>

extern "C"{
  #include "../../../lib/unp.h"
  #include <stdbool.h>
  #include <errno.h>
  #include <sys/time.h>
  #include <unistd.h>
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
	int					listenfd, connfd[4][100];
	pid_t				childpid;
	socklen_t			clilen[4];
	struct  sockaddr_in	      cliaddr[4], servaddr;
	char      sendline[MAXLINE], recvline[MAXLINE];
	char      id[4][32];
	bool      is_connected[4];
	Game game;
	

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
	/* Set socket receive timeout option */
	struct timeval accept_timeout = {6, 0};  // 6 seconds
	if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &accept_timeout, sizeof(accept_timeout)) < 0) {
	    perror("setsockopt(SO_RCVTIMEO) failed");
	    close(listenfd);
	    exit(1);
	}

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */
	
	
	int i = 0; // room index for child processes
	
	for (;;) {
	        /* parent process as acceptor */
	        
	        /* shuffle the play order */
	        std::vector<int> seatOrder = {0, 1, 2, 3};
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(seatOrder.begin(), seatOrder.end(), g);
	        
	        // reset connected[] flags
	        for(int j = 0; j <  4; j++){
		        is_connected[j] = false;
	        }
	        
		/* j-th client */
		for(int j = 0; j < 4;){
		  int seat_id = seatOrder[j];
		  
		  // accept  j-th client
		  clilen[seat_id] = sizeof(cliaddr[seat_id]);
		  if ( (connfd[seat_id][i] = accept(listenfd, (SA *) &cliaddr[seat_id], &clilen[seat_id])) < 0) {
		      if (errno == EINTR)
		          continue;		/* interrupted, back to next iteration of for() */
		      else{
		          printf("accept error: %s\n", strerror(errno)); 
		          continue;
		      }
		  }
		  
		  /* recv j-th client's id */
		  bzero(recvline, MAXLINE);
		  if (Read(connfd[seat_id][i], recvline, MAXLINE) == 0){
	              err_quit("tcp serv: client terminated prematurely");
	              continue;		/* interrupted, back to next iteration of for() */
		  }
		  
		  printf("debug: recieved msg: \"%s\"\n", recvline);
		  bzero(id[seat_id], 32);
		  sscanf(recvline, "%s", id[seat_id]);
		  
		  /* reply to j-th client */
		  bzero(sendline, MAXLINE);
		  sprintf(sendline, "Welcome to Big Two arena. You are the #%d player. Wait for other players!\n", seat_id + 1);
		  Writen(connfd[seat_id][i], sendline, strlen(sendline));
		  
		  // successfully add a client
		  is_connected[seat_id] = true;
		  j++;
                }
		/* end of parent accepting stage */
		
		/* game about to start */
                bzero(sendline, MAXLINE);
                sprintf(sendline, "Four players joined. Game is about to start!\n");
                printf("debug: broadcasting \"%s\"\n", sendline);
	        for(int j = 0; j < 4; j++){
	          Writen(connfd[j][i], sendline, strlen(sendline));
	        }
	        
		/* tell clients about each other*/
	        bzero(sendline, MAXLINE);
                sprintf(sendline, "The #1~4 players are: %s, %s, %s, %s.\n", id[0], id[1], id[2], id[3]);
                printf("debug: broadcasting \"%s\"\n", sendline);
	        for(int j = 0; j < 4; j++){
	          Writen(connfd[j][i], sendline, strlen(sendline));
	        }
	        
	        /* tell clients about starting player*/
	        game.startHumanGame();
	        
	        int cli_turn = game.findStartingPlayer();  // flag for who's turn to play
	        printf("debug: first player is #%d", cli_turn + 1);
	        
                bzero(sendline, MAXLINE);
                sprintf(sendline, "PLayer #%d %s holds 3C and starts the first play.\n", cli_turn + 1, id[cli_turn]);
                printf("debug: broadcasting \"%s\"\n", sendline);
                
	        for(int j = 0; j < 4; j++){
	          Writen(connfd[j][i], sendline, strlen(sendline));
	        }
		
                /* accept 4 clients successfully, fork() a child server */
		if ( (childpid = Fork()) == 0) {
			int			maxfdp1;
			fd_set		rset;
			Close(listenfd);	/* close listening socket */
	        
	                /* Initial board layout */
	                for(int j = 0; j < 4; j++){
	                  bzero(sendline, MAXLINE);
		          game.displayGameStateForPlayer(j, id[j], sendline);
		          printf("debug: sending to player#%d: \"%s\"", j + 1, sendline);
	                  Writen(connfd[j][i], sendline, strlen(sendline));
	                  
	                  if(cli_turn == j){
                              // 
                              bzero(sendline, MAXLINE);
                              sprintf(sendline, "Available combinations:\n");
                              Writen(connfd[j][i], sendline, strlen(sendline));
	                                    
                              // show combo
                              for(int k = 0; k < (int)game.getLegalActions(j).size(); k++){
                                  bzero(sendline, MAXLINE);
                                  sprintf(sendline, "[%d]", k);
                                  for(int l = 0; l < (int)game.getLegalActions(j)[k].size(); l++){
                                      strcat(sendline, " ");
                                      strcat(sendline, game.getLegalActions(j)[k][l].CardToString().c_str());
                                  }
                                  strcat(sendline, "\n");
                                      
                                  // send
                                  printf("debug: sending message to client#%d: %s\n", j + 1, sendline);
                                  Writen(connfd[j][i], sendline, strlen(sendline));
                              }
				            
	                      // enter msg
	                      bzero(sendline, MAXLINE);
	                      sprintf(sendline, "Enter a combination to play or pass: \n");
	                      printf("debug: sending message to client#%d: %s\n", j + 1, sendline);
	                      Writen(connfd[j][i], sendline, strlen(sendline));
                            }
	                }
			
			// game starts
			for ( ; ; ) {
			    // Turn on bits for 4 descriptors
		            FD_ZERO(&rset);
		            maxfdp1 = -1;
			    for(int j = 0; j < 4; j++){
                                if(is_connected[j]){
                                    FD_SET(connfd[j][i], &rset);
                                    maxfdp1 = max(maxfdp1, connfd[j][i] + 1);  // max descriptor + 1
	                        }
	                    }
	                    if(maxfdp1 < 0){
	                        // all clients have left, goto exit()
	                        break;
	                    }
			    Select(maxfdp1, &rset, NULL, NULL, NULL);
			    
			    cli_turn = game.getCurrentPlayer();
			    
			    for(int j = 0; j < 4; j++){
			        
			        if(j == cli_turn && game.passedRound[j] == false && !is_connected[cli_turn]){
			            // auto play/pass
			            game.passedRound[j] = true;
		                    game.nextTurn();
		                    cli_turn = game.getCurrentPlayer();
		                    // j has passed, check if two out of the other three has passed
	                            if((game.passedRound[(j+1)%4] && game.passedRound[(j+2)%4]) 
	                            || (game.passedRound[(j+2)%4] && game.passedRound[(j+3)%4])
	                            || (game.passedRound[(j+3)%4] && game.passedRound[(j+1)%4])){
	                                game.resetRound();
	                            }
			        }
			    
			        /* socket from client j is readable */
			        if ( is_connected[j] && FD_ISSET(connfd[j][i], &rset) ){
			            int n;  // read() return
				    bzero(recvline, MAXLINE);
				    n = Read(connfd[j][i], recvline, MAXLINE);
				    
				    // check read() return value
				    if(n <= 0){
					// get error
			                int error_code;
                                        socklen_t error_code_len = sizeof(error_code);
                                        Getsockopt(connfd[j][i], SOL_SOCKET, SO_ERROR, &error_code, &error_code_len);
                                        printf("socket error: %s\n", strerror(error_code));
                                        
                                        if(n < 0 || error_code == ECONNRESET || error_code == EPIPE){
                                            // ungraceful
                                            printf("str_serv: cli#%d has left ungracefully\n", j + 1);
			                }else{ // n == 0 && socket connected
			                    // graceful
                                            printf("str_serv: cli#%d is leaving gracefully\n", j + 1);
                                            
                                            /* reply to leaving client */
                                            bzero(sendline, MAXLINE);
                                            sprintf(sendline, "Bye!\n");
                                            printf("debug: sending message to client#%d: %s\n", j + 1, sendline);
                                            Writen(connfd[j][i], sendline, strlen(sendline));
                                            
                                            if(shutdown(connfd[j][i], SHUT_WR) < 0 && (error_code = errno) == ENOTCONN){
                                                // error occur after raed()
                                                printf("str_serv: cli#%d has left ungracefully\n", j + 1);
                                                printf("errorno: %s\n", strerror(error_code));
                                            }else{
                                                printf("str_serv: cli#%d has left gracefully\n", j + 1);
                                            }
			                }
			                
			                /* broadcast to other clients about the leaving*/
				        bzero(sendline, MAXLINE);
				        sprintf(sendline, "(player #%d \"%s\" left the room.)\n", j + 1, id[j]);
				        printf("debug: sending message to clients: %s\n", sendline);
				        
				        
				        for(int k = 0; k < 4; k++){
			                    if((k != j) && is_connected[k]){
			                        Writen(connfd[k][i], sendline, strlen(sendline));
				            }
				        }
				        
				        // set flags
				        is_connected[j] = false;
				        close(connfd[j][i]);
				    }else{
			                /* normal message */
				        printf("debug: message from client#%d: %s\n", j + 1, recvline);
				        
				        /* broadcast to other clients */
				        if(cli_turn == j){
				            // treat action from player
				            // char peek_num = recvline[0]; // peek 0(pass), 1(single), 2(pair), 5(5-combinations)
				            // HumanPlayer *this_player = dynamic_cast<HumanPlayer*>(game.getPlayers()[j].get());
				            // std::vector<Card> cards_to_play = this_player->playTurnAction(game.getLastPlay(), game.getLegalActions(j), true, connfd[j][i], sendline, recvline, is_connected);
				            
				            std::vector<Card> cards_to_play;
				            Combination this_combination;
				            
				            // if no combo available
				            if(game.getLegalActions(j).empty()){
				                bzero(sendline, MAXLINE);
		                                sprintf(sendline, "No legal actions. Forced PASS.\n");
				                printf("debug: sending message to client#%d: %s\n", j + 1, sendline);
				                
				                Writen(connfd[j][i], sendline, strlen(sendline));
				                game.passedRound[j] = true;
				                game.nextTurn();
				                cli_turn = game.getCurrentPlayer();
				                // j has passed, check if two out of the other three has passed
			                        if((game.passedRound[(j+1)%4] && game.passedRound[(j+2)%4]) 
			                        || (game.passedRound[(j+2)%4] && game.passedRound[(j+3)%4])
			                        || (game.passedRound[(j+3)%4] && game.passedRound[(j+1)%4])){
			                            game.resetRound();
			                        }
				                
				            }else{
			                        // treat received index
			                        int action = -1;
			                        sscanf(recvline, "%d", &action);
			                        
			                        if(action < (int)game.getLegalActions(j).size() && action >= 0){
			                            // legal action in range
			                            cards_to_play = game.getLegalActions(j)[action];
			                            this_combination = Combination(cards_to_play);
			                            
			                            // set play effect
			                            if(cards_to_play.size() != 0){
			                                game.checkValidPlay(cards_to_play);
			                                game.setLastPlay(this_combination);
			                            }
			                        }
			                        
			                        
			                        bzero(sendline, MAXLINE);
				                sprintf(sendline, "(player#%d \"%s\")", j + 1, id[j]);
				                if(cards_to_play.size() == 0){
				                    game.passedRound[j] = true;
				                    game.nextTurn();
				                    strcat(sendline, " PASS\n");
				                    // j has passed, check if two out of the other three has passed
				                    if((game.passedRound[(j+1)%4] && game.passedRound[(j+2)%4]) 
				                    || (game.passedRound[(j+2)%4] && game.passedRound[(j+3)%4])
				                    || (game.passedRound[(j+3)%4] && game.passedRound[(j+1)%4])){
				                        game.resetRound();
				                    }
				                }
				                for(int l = 0; l < (int)cards_to_play.size(); l++){
				                    strcat(sendline, " ");
				                    strcat(sendline, cards_to_play[l].CardToString().c_str());
				                }
				                strcat(sendline, "\n");
				                
				                // broadcast to others
				                printf("debug: sending message to clients: %s\n", sendline);
				                for(int k = 0; k < 4; k++){
				                    if(is_connected[k]){
			                                Writen(connfd[k][i], sendline, strlen(sendline));
				                    }
				                }
			                        
			                        // next player's turn
			                        // game.nextTurn();
			                        cli_turn = game.getCurrentPlayer();
				            }
				        }else{
				            // not his turn
				            printf("debug: not client#%d's turn\n", j + 1);
				            continue;
				        }
				    }
		                }
		                // if game is over
		                if(game.isGameOver()){
	                            /* broadcast game ending message */
	                            bzero(sendline, MAXLINE);
	                            sprintf(sendline, "Game is over. Congrats on our winner player#%d: %s!\n", j + 1, id[j]);
                                    printf("debug: broadcasting \"%s\"\n", sendline);
	                            for(int k = 0; k < 4; k++){
	                                Writen(connfd[k][i], sendline, strlen(sendline));
	                            }
	                            // score
	                            exit(0);
	                        }
			        /* next client */
			    }
	                    // update board state to players
	                    if(!game.isGameOver()){
	                        for(int j = 0; j < 4; j++){
		                    if(is_connected[j]){
		                        bzero(sendline, MAXLINE);
	                                game.displayGameStateForPlayer(j, id[j], sendline);
	                                Writen(connfd[j][i], sendline, strlen(sendline));
	                                if(cli_turn == j){
	                                    // 
	                                    bzero(sendline, MAXLINE);
	                                    sprintf(sendline, "Available combinations:\n");
	                                    Writen(connfd[j][i], sendline, strlen(sendline));
	                                    
	                                    // show combo
	                                    for(int k = 0; k < (int)game.getLegalActions(j).size(); k++){
				                bzero(sendline, MAXLINE);
		                                sprintf(sendline, "[%d]", k);
				                for(int l = 0; l < (int)game.getLegalActions(j)[k].size(); l++){
				                    strcat(sendline, " ");
				                    strcat(sendline, game.getLegalActions(j)[k][l].CardToString().c_str());
				                }
				                strcat(sendline, "\n");
				                
				                // send
				                printf("debug: sending message to client#%d: %s\n", j + 1, sendline);
				                Writen(connfd[j][i], sendline, strlen(sendline));
				            }
				            
				            // enter msg
				            bzero(sendline, MAXLINE);
				            sprintf(sendline, "Enter a combination to play or pass: \n");
				            printf("debug: sending message to client#%d: %s\n", j + 1, sendline);
				            Writen(connfd[j][i], sendline, strlen(sendline));
	                                }
		                    }
		                }
	                    }
		    }
		    /* all clients have terminated, exit */
		    exit(0);
		}
		
		/* parent closes connected socket */
		for(int j = 0; j < 4; j++){
		  Close(connfd[j][i]);
		}
		
		/* increment room number */
                i++;
                i = i % 100;  // assume no more than 100 rooms at a time
                
                /* start listening in the next room */
	}
}
