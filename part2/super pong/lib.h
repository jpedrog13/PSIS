#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include <ncurses.h>
#include <pthread.h>

#define WINDOW_SIZE 20
#define PADDLE_SIZE 2

//typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct ball_position_t{
    int x, y;
    int up_hor_down;            //  -1 up, 0 horizontal, 1 down
    int left_ver_right;         //  -1 left, 0 vertical,1 right
    char c;
} ball_position_t;

typedef struct paddle_position_t{
    int x, y;
    int length;
} paddle_position_t;

typedef struct message_c_t{
    int type;                   // 0 - connection;  1 - Paddle_move;
                                // 2 - não usado;   3 - disconnect       Consistência entre os 2 jogos
    int index;
    int paddle_dir;
}message_c_t;

/*typedef struct client_info_t {
    int index;
    int score;
    struct sockaddr_in client_addr;
}client_info_t;*/

typedef struct client_info_t{
    int index;
    int score;
    paddle_position_t paddle_pos;
    struct sockaddr_in client_addr;
}client_info_t;

typedef struct message_s_t{
    int index;
    int n_clients;
    ball_position_t ball_pos;
    client_info_t clients[10];
}message_s_t;



void place_ball_random(ball_position_t * ball);
void move_ball(ball_position_t * ball, paddle_position_t paddle, int paddle_index, client_info_t clients[10], int paddledir, int n_clients);
void draw_ball(WINDOW *win, ball_position_t * ball, int draw);
void random_paddle(paddle_position_t * paddle, client_info_t clients[10], int n_clients, int length, ball_position_t ball);
void draw_paddle(WINDOW *win, paddle_position_t paddle, int delete, int my_paddle);
void move_paddle(paddle_position_t * paddle, client_info_t clients[10], int n_clients, int direction, ball_position_t *ball, int index);
void print_scores(WINDOW *win, client_info_t clients[10], int n_clients, int my_index);
void print_empty_scores(WINDOW *win);
//void insert_last(client_list **head, int index, int score, paddle_position_t paddle_pos, struct sockaddr_in client_addr);
//void delete_index(client_list **head, int index);