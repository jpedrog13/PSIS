#define WINDOW_SIZE 20
#define PADDLE_SIZE 2

//typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct message_t{
    int type;                   // 0 - connection;  1 - send_ball/release_ball;
                                // 2 - move_ball;   3 - disconect
    ball_position_t ball_pos;
    paddle_position_t paddle_pos;
}message_t;

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
