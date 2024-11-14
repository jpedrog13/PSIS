#include "sock.h"
#include "lib.h"

typedef enum direction_t    {UP, DOWN, LEFT, RIGHT} direction_t;
typedef enum draw_t         {DELETE, DRAW}          draw_t;

WINDOW * my_win;
WINDOW * message_win;

//Declaration of useful variables
message_t m_client, m_server;
ball_position_t ball, old_ball;
paddle_position_t paddle;

int sock_fd;
struct sockaddr_in client_addr;
struct sockaddr_in server_addr;
socklen_t client_addr_size;
socklen_t server_addr_size;

pthread_mutex_t m1;

int key;
int play_state = 0;
int k;

//Thread to deal with messages received from the server
void* handle_comms(void *arg){
    while(1){
        recvfrom(sock_fd, &m_server, sizeof(m_server), 0,
                    (struct sockaddr *)&server_addr, &server_addr_size);
            
        pthread_mutex_lock(&m1);

        //Received a send ball message
        if(m_server.type == 1){
            //Changes play state to 1
            play_state = 1;
            m_client = m_server;
            //Draws new paddle position
            draw_paddle(my_win, &paddle, DRAW);

        }
        //Received a release ball message
        else if(m_server.type == 4){
            //Changes play state to 0
            play_state = 0;
            //Deletes previous paddle position
            draw_paddle(my_win, &paddle, DELETE);

        }
        //Received a move ball message
        else if(m_server.type == 2){           
            //Deletes previous ball position
            draw_ball(my_win,&m_client.ball_pos, DELETE);
            //Draws new ball position
            draw_ball(my_win, &m_server.ball_pos, DRAW);
            m_client.ball_pos = m_server.ball_pos;
           
        }       
        wrefresh(my_win);
        wrefresh(message_win);
        pthread_mutex_unlock(&m1);
    }
    return NULL;
}

//Thread to compute ball movement every second
void* handle_draw(void *arg){
    while(1){
        //When the client is playing compute the ball movement
        if(play_state){
            m_client.type=2;

            pthread_mutex_lock(&m1);
            //deletes previous ball position
            draw_ball(my_win,&m_client.ball_pos, DELETE);
            //computes new ball position
            move_ball(&m_client.ball_pos, paddle, key, message_win);
            //draws new paddle and ball positions
            draw_ball(my_win, &m_client.ball_pos, DRAW);

            mvwprintw(message_win, 1, 1, "PLAY STATE: %d", play_state);
            mvwprintw(message_win, 2, 1, "use arrow keys to");
            mvwprintw(message_win, 3, 1, "control paddle");

            wrefresh(my_win);
            wrefresh(message_win);
            
            //Sends new ball position to the server
            sendto(sock_fd, &m_client, sizeof(m_client), 0, 
                (const struct sockaddr *)&server_addr, sizeof(server_addr));            
            
            pthread_mutex_unlock(&m1);
            sleep(1);            
        }
        //When client is idle erase message window
        else{
            pthread_mutex_lock(&m1);
            mvwprintw(message_win, 1, 1, "              ");
            mvwprintw(message_win, 2, 1, "                   ");
            mvwprintw(message_win, 3, 1, "               ");
            wrefresh(message_win);
            pthread_mutex_unlock(&m1);
        }    
    }
    return NULL;
}

int main(int argc, char *argv[]){
    //Receives address from command line
    if(argc > 2) {
        printf("Too many arguments supplied.\n");
    }
    else if(argc < 2){
        printf("One argument expected.\n");
    }

    //Create Socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }

    //server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr) < 1){
		printf("no valid address: \n");
		exit(-1);
	}

    socklen_t client_addr_size = sizeof(struct sockaddr_in);
  
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
    my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0, 0);
    wrefresh(my_win);
    keypad(my_win, true);
    message_win = newwin(5, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);	
	wrefresh(message_win);
   

    //Declaration of useful variables
    int nbytes;
    int n = 0;
    int draw = 1;
    int delete = 0;
    //Creates a paddle
    new_paddle(&paddle, PADDLE_SIZE);

    pthread_mutex_init(&m1, NULL);

    //Declare threads
    pthread_t draw_thread_id;
    pthread_create(&draw_thread_id, NULL, handle_draw, NULL);
    pthread_t comm_thread_id;
    pthread_create(&comm_thread_id, NULL, handle_comms, NULL);

    //Infinite cycle while the player is in the game. Will terminate when 'q' is pressed
    do{
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
        //If the key is 'q', release the ball, send a disconnect message and end the program
        case 'q':
            pthread_mutex_lock(&m1);
            mvwprintw(message_win, 1, 1, "%d Disconnected from server", n);
            pthread_mutex_unlock(&m1);
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
        //If client has the ball compute paddle and ball position
        if (play_state){
            pthread_mutex_lock(&m1);
            //deletes previous paddle position
            draw_paddle(my_win, &paddle, DELETE);
            //deletes previous ball position
            draw_ball(my_win,&m_client.ball_pos, DELETE);
            //computes new paddle position
            move_paddle(&paddle, key, &m_client.ball_pos);
            //draws new paddle position
            draw_paddle(my_win, &paddle, DRAW);
            //draws new paddle and ball positions
            draw_ball(my_win, &m_client.ball_pos, DRAW);
            pthread_mutex_unlock(&m1);
            if (m_client.type == 2){
                sendto(sock_fd, &m_client, sizeof(m_client), 0, 
                    (const struct sockaddr *)&server_addr, sizeof(server_addr));
            }
        }
        //Otherwise deletes paddle
        else{
            pthread_mutex_lock(&m1);
            draw_paddle(my_win, &paddle, DELETE);
            pthread_mutex_unlock(&m1);
        }

    }while(key != 113);
    
    //Left the loop, disconnected
    //Sends a Disconnect message to the server informing
    m_client.type = 3;
    sendto(sock_fd, &m_client, sizeof(m_client), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));
    //Stop ncurses
    endwin();
    exit(0);
}