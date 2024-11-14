#include "lib.h"

//places ball pseudo-randomly in the board
void place_ball_random(ball_position_t * ball){
    ball->x = rand() % WINDOW_SIZE ;
    ball->y = rand() % WINDOW_SIZE ;
    ball->c = 'o';
    ball->up_hor_down = rand() % 3 -1;      // 0 horizontal, -1 up, 1 - down
    ball->left_ver_right = rand() % 3 -1 ;  // 0 vertical, -1 left, 1 right
}

//Computes the ball's next position
//We only compute collisions with the paddle if the ball hits the paddle vertically
//If we were to consider horizontal collisions, we would either bounce the ball back in the horizontal axis
//Or we would give the ball a random non-zero vertical speed
void move_ball(ball_position_t * ball, paddle_position_t paddle, int paddledir, WINDOW *win){
    //Next position on any given axis is given by current position plus velocity
    int next_x = ball->x + ball->left_ver_right;
    int next_y = ball->y + ball->up_hor_down;
    
    //If the ball hits a wall on the left or right, bounce it back
    if(next_x == 0 || next_x == WINDOW_SIZE-1 || ((next_x == paddle.x-paddle.length || 
        next_x == paddle.x+paddle.length) && next_y == paddle.y)){
        //If the ball is next to the top, make it go down
        if(ball->y==1){
            ball->up_hor_down = 1;
        }
        //If the ball is next to the bottom, make it go up
        else if(ball->y==WINDOW_SIZE){
            ball->up_hor_down = -1;
        }
        //Otherwise chose random direction
        else{
            ball->up_hor_down = rand() % 3 -1 ;
        }        
        ball->left_ver_right *= -1;
    }else{
        ball->x = next_x;
    }

    //Since the paddle updates first, check if paddle is currently on top of the ball
    if(ball->y == paddle.y && (ball->x >= paddle.x - paddle.length && ball->x <= paddle.x + paddle.length)){
        //If both the paddle and the ball are moving in opposite directions, change the ball's direction
        if(paddledir == KEY_UP && ball->up_hor_down == 1){
            ball->up_hor_down *= -1;
        }
        else if(paddledir == KEY_DOWN && ball->up_hor_down == -1){
            ball->up_hor_down *= -1;
        }
        //Otherwise, the ball keeps the same direction

        //Update ball
        ball->y = ball->y + ball->up_hor_down;
    }
    //If the ball hits a wall on the bottom or top, or hits a paddle, change the ball's direction
    else if(next_y == 0 || next_y == WINDOW_SIZE-1 || (next_y == paddle.y && (next_x >= paddle.x - paddle.length && 
            next_x <= paddle.x + paddle.length))) {
        //If the ball is next to the left wall, make it go right
        if(ball->x == 1){
            ball->left_ver_right = 1;
        }
        //If the ball is next to the right wall, make it go left
        else if(ball->x == WINDOW_SIZE-2){
            ball->left_ver_right = -1;
        }
        //Otherwise chose random direction
        else{
            ball->left_ver_right = rand() % 3 -1;
        }
        ball->up_hor_down *= -1;
    }else{
        ball->y = next_y;
    }
}

//Draws or deletes the ball
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

//Computes the first paddle position
void new_paddle (paddle_position_t * paddle, int length){
    paddle->x = WINDOW_SIZE/2;
    paddle->y = WINDOW_SIZE-2;
    paddle->length = length;
}

//Draws or deletes the paddle
void draw_paddle(WINDOW *win, paddle_position_t * paddle, int delete){
    int ch;
    if(delete){
        ch = '_';
    }else{
        ch = ' ';
    }
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    for (int x = start_x; x <= end_x; x++){
        wmove(win, paddle->y, x);
        waddch(win,ch);
    }
    wrefresh(win);
}

//Computes the next paddle position based on input direction
void move_paddle(paddle_position_t * paddle, int direction, ball_position_t * ball){
    //If the paddle moves up
    if (direction == KEY_UP){
        //Check if paddle is next to the top, or if the paddle is making the ball go over the top
        if (paddle->y  != 1 && !(paddle->y == 2 && ball->y==1 && (ball->x >= paddle->x - paddle->length 
            && ball->x <= paddle->x + paddle->length))){
            paddle->y --;
        }
        //If the paddle hits the ball
        if(ball->y==paddle->y && (ball->x >= paddle->x - paddle->length && ball->x <= paddle->x + paddle->length)){
            //If ball is moving down make it go up and change movement direction
            if (ball->up_hor_down==1 || ball->up_hor_down==0){
                ball->up_hor_down=-1;
                ball->y --;
            }
            //Otherwise just make the ball move up
            else{
                ball->y --;
            }
        }
    }
    //If the paddle moves down
    if (direction == KEY_DOWN){
        //Check if paddle is next to the bottom, or if the paddle is making the ball go below the bottom
        if (paddle->y  != WINDOW_SIZE-2 && !(paddle->y == WINDOW_SIZE-3 && ball->y==WINDOW_SIZE-2 && 
            (ball->x >= paddle->x - paddle->length && ball->x <= paddle->x + paddle->length))){
            paddle->y ++;
        }
        //If the paddle hits the ball
        if(ball->y==paddle->y && (ball->x >= paddle->x - paddle->length && ball->x <= paddle->x + paddle->length)){
            //If ball is moving up make it go down and change movement direction
            if (ball->up_hor_down==-1 || ball->up_hor_down==0){
                ball->up_hor_down=1;
                ball->y ++;
            }
            //Otherwise just make the ball move down
            else{
                ball->y ++;
            }
        }
    }
    //If the paddle moves left
    if (direction == KEY_LEFT){
        //Check if paddle is next to the right wall, or if the paddle is making the ball go over the wall
        if (paddle->x - paddle->length != 1 && !(paddle->x - paddle->length == 2 
            && ball->x == 1 && ball->y == paddle->y)){
            paddle->x --;
        }
        //If the paddle hits the ball
        if(ball->x==paddle->x-paddle->length && ball->y == paddle->y){
            //If ball is moving right make it go left and change movement direction
            if (ball->left_ver_right==1 || ball->left_ver_right==0){
                ball->left_ver_right = -1;
                ball->x --;
            }
            //Otherwise just make the ball move left
            else{
                ball->x --;
            }
        }
    }
    //If the paddle moves right
    if (direction == KEY_RIGHT){
        //Check if paddle is next to the right wall, or if the paddle is making the ball go over the wall
        if (paddle->x + paddle->length != WINDOW_SIZE-2 && !(paddle->x + paddle->length == WINDOW_SIZE-3 
            && ball->x == WINDOW_SIZE-2 && ball->y == paddle->y)){
            paddle->x ++;
        }
        //If the paddle hits the ball
        if(ball->x==paddle->x+paddle->length && ball->y == paddle->y){
            //If ball is moving left make it go right and change movement direction
            if (ball->left_ver_right==-1 || ball->left_ver_right==0){
                ball->left_ver_right = 1;
                ball->x ++;
            }
            //Otherwise just make the ball move right
            else{
                ball->x ++;
            }
        }
    }
}

//We would use this if we were to save data in a linked list, but we don't

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