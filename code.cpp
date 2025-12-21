#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Boat.h"

SDL_Texture* ship;
SDL_Rect rect;
SDL_Point center = {225 / 2, 105 / 2};


int GetAxisY(){
	const Uint8 *keyState = SDL_GetKeyboardState(NULL);
	return std::max(-1, std::min((keyState[SDL_SCANCODE_S] - keyState[SDL_SCANCODE_W] + keyState[SDL_SCANCODE_DOWN] - keyState[SDL_SCANCODE_UP]), 1));
}

int GetAxisX(){
	const Uint8 *keyState = SDL_GetKeyboardState(NULL);
	return std::max(-1, std::min((keyState[SDL_SCANCODE_D] - keyState[SDL_SCANCODE_A] + keyState[SDL_SCANCODE_RIGHT] - keyState[SDL_SCANCODE_LEFT]), 1));
}



BoatSimulator boat(50000, 10, 3, 1, 3, 0.0, 2.0);
double uThrust = 1.0;
double uAngle = 1.0;
Vector2D wind = {0, 10}; 
Vector2D flow = {0, 0};
double timeScale = 0.2;

int i = 0;



void start(SDL_Window *window, SDL_Renderer *renderer){
	ship = IMG_LoadTexture(renderer, "Assets/ship.png");	
	SDL_SetRenderDrawColor(renderer, 99, 155, 255, 255);
}

void loop(SDL_Window *window, SDL_Renderer *renderer){
	uThrust = fmin(fmax(uThrust - GetAxisY() * 0.1, -5.0),5);
	uAngle = fmod(uAngle - GetAxisX(), 360.0);

	boat.update(0.1 * timeScale, uThrust, uAngle, wind, flow);

	std::cout << "Т=" << i*0.1 << "с | "
		<< "Поз: (" << boat.position.x << ", " << boat.position.y << ") | "
		<< "Штурвал: (" << uAngle << ") | "
		<< "Тяга: (" << uThrust << ") | "
		<< "Курс: " << boat.angle * 180.0 / 3.14159265358979323846 << " град." << std::endl;


    rect = {int(boat.position.x * 100), int(boat.position.y * 100), 225, 105};
	SDL_RenderCopyEx(renderer, ship, NULL, &rect, boat.angle  * 180.0 / 3.14159265358979323846, &center, SDL_FLIP_NONE);

	i++;
}
