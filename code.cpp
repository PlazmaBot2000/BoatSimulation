#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <bits/stdc++.h>
#include "Boat.h"



SDL_Texture* ship;
SDL_Rect rect;
SDL_Point center = {225 / 2, 105 / 2};



BoatSimulator myBoat(50000.0, 10.0, 3.0, 1.0, 2.0);
double deltaTime = 0.01;
Vector2D windVelocity = {5.0, 2.0};
Vector2D currentVelocity = {0.0, 0.0};
Vector2D thrust = {5000.0, 0.0};
int i = 0;


void info(){
	std::cout << "Time: " << i * deltaTime << "s, Position: (" << myBoat.position.x << ", " 
		<< myBoat.position.y << ") m, Velocity: (" << myBoat.velocity.x << ", " << myBoat.velocity.y
		<< ") m/s, Velocity magnitude: " << myBoat.velocity.magnitude() << " m/s." << std::endl;

}

int GetAxisY(){
	const Uint8 *keyState = SDL_GetKeyboardState(NULL);
	return std::max(-1, std::min((keyState[SDL_SCANCODE_S] - keyState[SDL_SCANCODE_W] + keyState[SDL_SCANCODE_DOWN] - keyState[SDL_SCANCODE_UP]), 1));
}

int GetAxisX(){
	const Uint8 *keyState = SDL_GetKeyboardState(NULL);
	return std::max(-1, std::min((keyState[SDL_SCANCODE_D] - keyState[SDL_SCANCODE_A] + keyState[SDL_SCANCODE_RIGHT] - keyState[SDL_SCANCODE_LEFT]), 1));
}

void start(SDL_Window *window, SDL_Renderer *renderer){
	ship = IMG_LoadTexture(renderer, "ship.png");	
	SDL_SetRenderDrawColor(renderer, 99, 155, 255, 255);
}

void loop(SDL_Window *window, SDL_Renderer *renderer){

	thrust.x += GetAxisX() * 100.0;
	thrust.y += GetAxisY() * 100.0;

	myBoat.update(deltaTime, windVelocity, currentVelocity, thrust);

    rect = {int(myBoat.position.x * 100), int(myBoat.position.y * 100), 225, 105};
	SDL_RenderCopyEx(renderer, ship, NULL, &rect, int(std::atan2(myBoat.velocity.y, myBoat.velocity.x) * 180.0 / 3.14), &center, SDL_FLIP_NONE);

	i++;
}
