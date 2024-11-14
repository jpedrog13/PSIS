#include "lib.h"
#include "sock.h"

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

int main(int argc, char *argv[]){
    
    //Receives address from command line    
    if( argc > 2 ) {
        printf("Too many arguments supplied.\n");
        exit(0);
    }
    else if(argc < 2) {
        printf("One argument expected.\n");
        exit(0);
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

    //Declare useful variables
    message_c_t m_client;
    message_s_t m_server;
    ball_position_t ball;
    paddle_position_t paddle;

    //Setup connection message
    m_client.type = 0;
    m_client.index = 0;
    m_client.paddle_dir = 0;
    
    //Send connection message
    sendto(sock_fd, &m_client, sizeof(m_client), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));

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
    /* creates a window and draws a border */
    WINDOW * message_win = newwin(12, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);
	wrefresh(message_win);


    client_info_t clients_old[10];
    int n_clients_old;

    int key;
    int n = 0;
    int first = 1;
    int my_index;
    //Infinite loop to get messages from the server
    do{
        recvfrom(sock_fd, &m_server, sizeof(m_server), 0,
                    (struct sockaddr *)&client_addr, &client_addr_size);

        //If the server send a "Server is full" message, exit the program
        if(m_server.index < 0){
            mvwprintw(message_win, 1, 1, "Server is full. Exiting.", n);
            wrefresh(message_win);
            break;
        }

        //If this is the first time in the loop, initiate useful variables
        else if(first){
            my_index = m_server.index;
            m_client.index = my_index;
            first = 0;
            n_clients_old = m_server.n_clients;
            for(int i = 0; i < n_clients_old; i++){
                clients_old[i].paddle_pos = m_server.clients[i].paddle_pos;
            }
        }
        //Clean-up the board of old paddles and old board position
        for (int i = 0; i < n_clients_old; i++){
            draw_paddle(my_win, clients_old[i].paddle_pos, FALSE, FALSE);
        }
        draw_ball(my_win, &ball, FALSE);

        //Redraw paddles and ball
        for (int i = 0; i < m_server.n_clients; i++){
            if(m_server.clients[i].index == my_index){
                draw_paddle(my_win, m_server.clients[i].paddle_pos, TRUE, TRUE);
                
            }
            else{
                draw_paddle(my_win, m_server.clients[i].paddle_pos, TRUE, FALSE);
            }
        }
        draw_ball(my_win, &m_server.ball_pos, TRUE);
        wrefresh(my_win);
        wrefresh(message_win);

        //Clear scoreboard window
        print_empty_scores(message_win);
        //Print scores on scoreboard window
        print_scores(message_win, m_server.clients, m_server.n_clients, my_index);

        m_client.paddle_dir = 0;
        //Check for directional or quit inputs
        key = wgetch(my_win);
        if(isalpha(key)){key = tolower(key);}
        n++;
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
        //Update variables that store old ball and clients info
        ball = m_server.ball_pos;
        n_clients_old = m_server.n_clients;
        for(int i = 0; i < n_clients_old; i++){
            clients_old[i].paddle_pos = m_server.clients[i].paddle_pos;
        }
        
        //Send Paddle_move message to server
        sendto(sock_fd, &m_client, sizeof(m_client), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));
    }while(key != 113);
    endwin();
    return 0;
}