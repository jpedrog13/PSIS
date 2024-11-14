#include "lib.h"
#include "sock.h"

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

int sock_fd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
socklen_t client_addr_size, server_addr_size;

pthread_mutex_t m1;

//Declare useful variables
message_c_t m_client;
message_s_t m_server;
ball_position_t ball;
paddle_position_t paddle;

WINDOW * my_win;
WINDOW * message_win;

client_info_t clients_old[10];

int n_clients_old;
int key;
int n;
int first;
int my_index;
int nbytes;

//Thread that continually reads from the server to update paddle and ball positions
void * handle_comms(void *arg){
    while(1){
        nbytes = read(sock_fd, &m_server, sizeof(m_server));
        if(nbytes > 0){
            //First message received from server saves is index
            if(first){
                my_index = m_server.index;
                m_client.index = my_index;
                first = 0;
                n_clients_old = m_server.n_clients;
                for(int i = 0; i < 10; i++){
                    if (m_server.clients[i].score!=-1){
                        clients_old[i].paddle_pos = m_server.clients[i].paddle_pos;
                    }
                }
            }
            else{
                pthread_mutex_lock(&m1);
                //Delete old paddle positions
                for (int i = 0; i < 10; i++){
                    if (clients_old[i].score!=-1){
                        draw_paddle(my_win, clients_old[i].paddle_pos, FALSE, FALSE);
                    }
                }
                //Delete old ball position
                draw_ball(my_win, &ball, FALSE);

                //Redraw paddles and ball
                for (int i = 0; i < 10; i++){
                    if (m_server.clients[i].score!=-1){
                        if(m_server.clients[i].index == my_index){
                            draw_paddle(my_win, m_server.clients[i].paddle_pos, TRUE, TRUE);
                            
                        }
                        else{
                            draw_paddle(my_win, m_server.clients[i].paddle_pos, TRUE, FALSE);
                        }
                    }
                }
                draw_ball(my_win, &m_server.ball_pos, TRUE);
                //Update paddle and ball positions
                ball = m_server.ball_pos; 
                n_clients_old = m_server.n_clients;
                for(int i = 0; i < 10; i++){
                    if (m_server.clients[i].score!=-1){
                        clients_old[i].paddle_pos = m_server.clients[i].paddle_pos;
                    }
                }
                wrefresh(my_win);
                //Clear scoreboard window
                print_empty_scores(message_win);
                //Print scores on scoreboard window
                print_scores(message_win, m_server.clients, m_server.n_clients, my_index);
                wrefresh(message_win);
                pthread_mutex_unlock(&m1);
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]){
    
    //Receives address from command line    
    if(argc > 2) {
        printf("Too many arguments supplied.\n");
        exit(0);
    }
    else if(argc < 2) {
        printf("One argument expected.\n");
        exit(0);
    }
    
    //Create Socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    server_addr_size = sizeof(struct sockaddr);
    client_addr_size = sizeof(struct sockaddr);

    if(connect(sock_fd, (struct sockaddr*)&server_addr, server_addr_size) == 0){
        printf("[+]Connected\n");
    }else{
        printf("[-]Connection failed, exiting...\n");
        perror("connect");
        exit(-1);
    }

    //Setup connection message
    m_client.type = 0;
    m_client.index = 0;
    m_client.paddle_dir = 0;

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
    /* creates a window and draws a border */
    message_win = newwin(12, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);
	wrefresh(message_win);

    first = 1;
    int n = 0;

    pthread_mutex_init(&m1, NULL);

    pthread_t comms_thread_id;
    pthread_create(&comms_thread_id, NULL, handle_comms, NULL);
    
    //Infinite loop to get messages from the server
    do{


        //If this is the first time in the loop, initiate useful variables
        while(first);
        //Clean-up the board of old paddles and old board position
        m_client.paddle_dir = 0;
        //Check for directional or quit inputs
        key = wgetch(my_win);
        if(isalpha(key)){key = tolower(key);}
        switch(key){
        case KEY_LEFT:
            m_client.type = 1;
            m_client.paddle_dir = key;
            break;
        case KEY_RIGHT:
            m_client.type = 1;
            m_client.paddle_dir = key;
            break;
        case KEY_UP:
            m_client.type = 1;
            m_client.paddle_dir = key;
            break;
        case KEY_DOWN:
            m_client.type = 1;
            m_client.paddle_dir = key;
            break;
        case 'q':
            m_client.type = 3;
            break;
        default:
            key = 'x';
            break;
        }
        //Send Paddle_move message to server
        nbytes = write(sock_fd, &m_client, sizeof(m_client));
    }while(key != 113);
    endwin();
    exit(0);
}