#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <ctype.h> 
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>

#include <ncurses.h>

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

typedef struct message_t{
    int type;                   // 0 - connection;  1 - send_ball;
                                // 2 - move_ball;   3 - disconnect;  4 - release_ball
    ball_position_t ball_pos;
    paddle_position_t paddle_pos;
}message_t;


typedef struct client_info_t {
    int play_state;
    struct sockaddr_in client_addr;
}client_info_t;

void place_ball_random(ball_position_t * ball);
void move_ball(ball_position_t * ball, paddle_position_t paddle, int paddledir, WINDOW *win);
void draw_ball(WINDOW *win, ball_position_t * ball, int draw);
void new_paddle (paddle_position_t * paddle, int length);
void draw_paddle(WINDOW *win, paddle_position_t * paddle, int delete);
void move_paddle(paddle_position_t * paddle, int direction, ball_position_t * ball);
//void insert_last(client_list **head, int index, int score, paddle_position_t paddle_pos, struct sockaddr_in client_addr);
//void delete_index(client_list **head, int index);