#include <ncurses.h>
#include "ntetris.h"

void move_tetrimino (TETRIMINO *tetrimino, int direction) 
{

}

/* Rotates the tetrimino about its pivot coordinates
(the coordinates of one of the four circles that make up the tetrimino)

WARNING: need to handle case where user attempts to rotate piece near edge of well */

void rotate_tetrimino (TETRIMINO *tetrimino) 
{
	int temp;
	for (int i = 0; i < 4; i++)
	{
		tetrimino->bits[i].y -= tetrimino->pivot.y;
		tetrimino->bits[i].x -= tetrimino->pivot.x;

		temp = tetrimino->bits[i].y; 
		tetrimino->bits[i].y =  !tetrimino->bits[i].x + 1;
		tetrimino->bits[i].x = temp;

		tetrimino->bits[i].y += tetrimino->pivot.y;
		tetrimino->bits[i].x += tetrimino->pivot.x;
	}
}



void init_tetrimino (TETRIMINO *tetrimino, int tetrimino_id)
{

}