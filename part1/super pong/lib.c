#include "lib.h"

//places ball pseudo-randomly in the board
void place_ball_random(ball_position_t * ball){
    ball->x = rand() % WINDOW_SIZE;
    ball->y = rand() % WINDOW_SIZE;
    ball->c = 'o';
    ball->up_hor_down = rand() % 3 -1; // 0 horizontal, -1 up, 1 - down
    ball->left_ver_right = rand() % 3 -1 ; // 0 vertical, -1 left, 1 right
}

//Computes the ball's next position
//We only compute collisions with the paddle if the ball hits the paddle vertically
//If we were to consider horizontal collisions, we would either bounce the ball back in the horizontal axis
//Or we would give the ball a random non-zero vertical speed
void move_ball(ball_position_t * ball, paddle_position_t paddle, int paddle_index, client_info_t clients[10], int paddledir, int n_clients){
    int collided = 0;
    int next_x = ball->x + ball->left_ver_right;
    //If the ball hits a wall on the left or right, bounce it back
    if(next_x == 0 || next_x == WINDOW_SIZE-1){
        collided = 1;
        ball->up_hor_down = rand() % 3 -1 ;
        ball->left_ver_right *= -1;
    }else{
        ball->x = next_x;
    }
    
    int next_y = ball->y + ball->up_hor_down;
    //Since the paddle updates first, check if paddle is currently on top of the ball
    if(ball->y == paddle.y && (ball->x >= paddle.x - paddle.length && ball->x <= paddle.x + paddle.length)){
        //If both the paddle and the ball are moving in opposite directions, change the ball's direction
        if(paddledir == KEY_UP && ball->up_hor_down == 1){
            collided = 1;
            ball->up_hor_down *= -1;
        }
        else if(paddledir == KEY_DOWN && ball->up_hor_down == -1){
            collided = 1;
            ball->up_hor_down *= -1;
        }
        //Otherwise, the ball keeps the same direction
        //Update ball
        ball->y = ball->y + ball->up_hor_down;
    }
    //If the ball hits a paddle, change the ball's direction
    else if(next_y == paddle.y && (next_x >= paddle.x - paddle.length && next_x <= paddle.x + paddle.length)) {
        clients[paddle_index].score++;
        collided = 1;
        ball->up_hor_down *= -1;
        ball->left_ver_right = rand() % 3 - 1;
    }
    //If the ball hits a side wall, change it's direction
    else if(next_y == 0 || next_y == WINDOW_SIZE-1){
        collided = 1;
        ball->up_hor_down *= -1;
        ball->left_ver_right = rand() % 3 - 1;
    }
    //If the ball hits a different paddle, bounce off it
    else{
        for(int i = 0; i < n_clients; i++){
            if(clients[i].index != paddle_index && next_y == clients[i].paddle_pos.y &&
                (next_x >= clients[i].paddle_pos.x - paddle.length && next_x <= clients[i].paddle_pos.x + paddle.length)){
                clients[i].score++;
                collided = 1;
                ball->up_hor_down *= -1;
                //next_y = ball->y + ball->up_hor_down;
                ball->left_ver_right = rand() % 3 - 1;
                break;
            }
        }
    }
    if(!collided){
        ball->y = next_y;
    }
}

void draw_ball(WINDOW *win, ball_position_t * ball, int draw){
    int ch;
    if(draw){
        ch = ball->c;
    }else{
        ch = ' ';
    }
    wmove(win, ball->y, ball->x);
    waddch(win,ch);
    wrefresh(win);
}

//Place pseudo-random paddle where there isn't a paddle already
void random_paddle(paddle_position_t * paddle, client_info_t clients[10], int n_clients, int length){
    int failed = 1;
    int changed;
    paddle->x = rand() % (WINDOW_SIZE - 6) + 3;
    paddle->y = rand() % (WINDOW_SIZE - 1) + 1;
    paddle->length = length;

    if (n_clients > 0){
        while(failed){
            changed = 0;
            for(int i = 0; i < n_clients; i++){
                if(paddle->y == clients[i].paddle_pos.y){
                    paddle->y = rand() % (WINDOW_SIZE - 1) + 1;
                    changed = 1;
                    break;
                }
            }
            if (!changed)
                failed = 0;
        }
    }
}

void draw_paddle(WINDOW *win, paddle_position_t paddle, int delete, int my_paddle){
    int ch;
    if(my_paddle && delete){
        ch = '=';
    }else if(delete){
        ch = '_';
    }else{
        ch = ' ';
    }
    int start_x = paddle.x - paddle.length;
    int end_x = paddle.x + paddle.length;
    for (int x = start_x; x <= end_x; x++){
        wmove(win, paddle.y, x);
        waddch(win,ch);
    }
    wrefresh(win);
}

//Move Paddle and don't run into other paddles
void move_paddle(paddle_position_t * paddle, client_info_t clients[10], int n_clients, int direction){
    int length = paddle->length;
    if (direction == KEY_UP){
        if (paddle->y  != 1){
            for (int i = 0; i < n_clients; i++){
                if(paddle->y - 1 == clients[i].paddle_pos.y && 
                ((paddle->x - length <= clients[i].paddle_pos.x + length && paddle->x - length >= clients[i].paddle_pos.x - length) ||
                 (paddle->x + length >= clients[i].paddle_pos.x - length && paddle->x + length <= clients[i].paddle_pos.x + length)))
                    return;
            }
            paddle->y--;
        }
    }
    if (direction == KEY_DOWN){
        if (paddle->y  != WINDOW_SIZE-2){
            for (int i = 0; i < n_clients; i++){
                if(paddle->y + 1 == clients[i].paddle_pos.y && 
                ((paddle->x - length <= clients[i].paddle_pos.x + length && paddle->x - length >= clients[i].paddle_pos.x - length) ||
                 (paddle->x + length >= clients[i].paddle_pos.x - length && paddle->x + length <= clients[i].paddle_pos.x + length)))
                    return;
            }
            paddle->y++;
        }
    }
    if (direction == KEY_LEFT){
        if (paddle->x - length != 1){
            for (int i = 0; i < n_clients; i++){
                if(paddle->x - length - 1 == clients[i].paddle_pos.x + length && paddle->y == clients[i].paddle_pos.y)
                    return;
            }
            paddle->x--;
        }
    }
    if (direction == KEY_RIGHT){
        if (paddle->x + length != WINDOW_SIZE-2){
            for (int i = 0; i < n_clients; i++){
                if(paddle->x + length + 1 == clients[i].paddle_pos.x - length && paddle->y == clients[i].paddle_pos.y)
                    return;
            }
            paddle->x++;
        }
    }
}

//Print scores on window
void print_scores(WINDOW *win, client_info_t clients[10], int n_clients, int my_index){
    for (int i = 0; i < n_clients; i++){
        if (clients[i].index == my_index){
            mvwprintw(win, i + 1, 1, "P%d - %d <---", i+1, clients[i].score);
        }
        else{
            mvwprintw(win, i + 1, 1, "P%d - %d     ", i+1, clients[i].score);
        }
    }
    wrefresh(win);
}

void print_empty_scores(WINDOW *win){
    for(int i = 1; i<11; i++){
        mvwprintw(win, i, 1, "                           ");
    }
}

/*void insert_last(client_list **head, int index, int score, paddle_position_t paddle_pos, struct sockaddr_in client_addr){
    
    client_list* new = (client_list*) malloc(sizeof(client_list));
    client_list *aux;

    new->index = index;
    new->score = score;
    new->paddle_pos = paddle_pos;
    new->client_addr = client_addr;
    new->next = NULL;

    if(*head == NULL){
        
        *head = new;
        return;
    }
    else{
        aux = *head;
        while(aux->next != NULL){
            aux = aux->next;
        }
        aux->next = new;
    }
    return;
}

void delete_index(client_list **head, int index){
    if(*head == NULL){
        return;
    }
    client_list *aux = *head;
    client_list *aux2 = NULL;

    if((*head)->index == index){
        *head = (*head)->next;
        free(aux);
        return;
    }
    while(aux->next->index != index  && aux->next != NULL){
        aux = aux->next;
    }
    if(aux->next->index == index){
        aux2 = aux->next->next;
        free(aux->next);
        aux->next = aux2;
    }
    return;
}*/