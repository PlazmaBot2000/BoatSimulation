#include <SDL2/SDL.h>
#ifndef MODULES_H
#define MODULES_H

class Engine_GetAxis{
	public:
		static int X();
		static int Y();
};

class Collider {
    float x, y; // Позиция
    float width, height; // Размеры
	bool checkCollision(Collider b) {
    	return (x < b.x + b.width &&
            x + width > b.x &&
            y < b.y + b.height &&
            y + height > b.y);
	}
};


bool Engine_Delay(int time, int& Prev_Tick);
int Engine_Random(int upper, int lower);
#endif
