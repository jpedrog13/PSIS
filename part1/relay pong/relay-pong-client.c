#include "sock.h"
#include "lib.h"

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

int main(int argc, char *argv[]){
    //Receives address from command line
    if( argc > 2 ) {
        printf("Too many arguments supplied.\n");
    }
    else if(argc < 2){
        printf("One argument expected.\n");
    }

    //Create Socket
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr) < 1){
		printf("no valid address: \n");
		exit(-1);
	}
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);

    //Declaration of useful variables
    message_t m_client, m_server;
    ball_position_t ball;
    paddle_position_t paddle;
    
    //Setup Connection message
    m_client.type = 0;
    m_client.ball_pos = ball;


    //Send Connection message
    sendto(sock_fd, &m_client, sizeof(m_client), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("message sent");

    //Initialize ncurses
    initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();


    //Create 2 separate windows
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0, 0);
    wrefresh(my_win);
    keypad(my_win, true);
    WINDOW * message_win = newwin(5, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);	
	wrefresh(message_win);
   

    //Declaration of useful variables
    int nbytes;
    int key = 0;
    int n = 0;
    int draw = 1;
    int play_state = 0;
    int delete = 0;
    //Creates a paddle
    new_paddle(&paddle, PADDLE_SIZE);

    //Infinite cycle while the player is in the game. Will terminate when 'q' is pressed
    do{
        //Infinite cycle while player is in control
        while(play_state && key != 113){
            //Waits for the next key movement
            key = wgetch(my_win);
            if (isalpha(key)){key = tolower(key);}
            n++;
            switch(key){
            //If it's an arrow key, process movement
            case KEY_LEFT:
                m_client.type = 2;
                break;
            case KEY_RIGHT:
                m_client.type = 2;
                break;
            case KEY_UP:
                m_client.type = 2;
                break;
            case KEY_DOWN:
                m_client.type = 2;
                break;
            //If the key is 'r', release the ball and exit cycle (Play state = 0)
            case 'r':
                m_client.type = 1;
                play_state = 0;
                break;
            //If the key is 'q', release the ball, send a disconnect message and end the program
            case 'q':
                mvwprintw(message_win, 1, 1, "%d Disconnected from server", n);
                m_client.type = 1;
                sendto(sock_fd, &m_client, sizeof(m_client), 0, 
                        (const struct sockaddr *)&server_addr, sizeof(server_addr));
                m_client.type = 3;
                play_state = 0;
                break;
            default:
                key = 'x';
                break;
            }
            //deletes previous paddle position
            draw_paddle(my_win, &paddle, delete);
            //computes new paddle position
            move_paddle(&paddle, key);
            //deletes previous ball position
            draw_ball(my_win,&m_client.ball_pos, delete);
            //computes new ball position
            move_ball(&m_client.ball_pos, paddle, key, message_win);
            //draws new paddle and ball positions
            draw_paddle(my_win, &paddle, draw);
            draw_ball(my_win, &m_client.ball_pos, draw);

            //Write relevant messages on screen when player is in control
            if(play_state){
                mvwprintw(message_win, 1, 1, "PLAY STATE");
                mvwprintw(message_win, 2, 1, "use arrow keys to");
                mvwprintw(message_win, 3, 1, "control paddle");
            }
            //Wipe out the message board when player loses control
            else{
                mvwprintw(message_win, 1, 1, "           ");
                mvwprintw(message_win, 2, 1, "                   ");
                mvwprintw(message_win, 3, 1, "               ");
            }


            //Send updated ball and paddle positions to the server
            //Release_ball messages and Move_ball messages
            if (m_client.type == 1 || m_client.type == 2){
                sendto(sock_fd, &m_client, sizeof(m_client), 0, 
                (const struct sockaddr *)&server_addr, sizeof(server_addr));
            }
            wrefresh(my_win);
            wrefresh(message_win);
        }
        //Infinite cycle while player waits to get control
        while(!play_state && key != 113){
            //Waits for new message from the server
            recvfrom(sock_fd, &m_server, sizeof(m_server), 0,
                    (struct sockaddr *)&client_addr, &client_addr_size);

            //If the player receives a Send_ball message
            //Gains control of the ball
            if(m_server.type == 1){
                //Changes play state to 1 ending the cycle
                play_state = 1;
                //Deletes previous paddle and ball positions
                draw_paddle(my_win, &paddle, delete);
                draw_ball(my_win, &m_client.ball_pos, delete);
            }
            //Updates the ball positions
            else if(m_server.type == 2){
                draw_ball(my_win, &m_client.ball_pos, delete);
                draw_ball(my_win, &m_server.ball_pos, draw);
            }
            
            //Update the last ball position and store it for
            //Clean-up purposes
            m_client.ball_pos = m_server.ball_pos;

            //Refresh ncurses windows
            wrefresh(my_win);
            wrefresh(message_win);
        }
    }while(key != 113);
    
    //Left the loop, disconnected
    //Sends a Disconnect message to the server informing
    m_client.type = 3;
    sendto(sock_fd, &m_client, sizeof(m_client), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));
    //Stop ncurses
    endwin();
    return 0;
}