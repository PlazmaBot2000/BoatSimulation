#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Boat.h"
#include "Engine/Engine_Modules.h"
#include <toml++/toml.hpp>



SDL_Texture* ship;
SDL_Rect rect;
SDL_Point center = {180 / 2, 99 / 2};

auto config = toml::parse_file("config.toml");

BoatSimulator boat(config["ship"]["mass"].value_or(50000), config["ship"]["length"].value_or(10),
		config["ship"]["beam"].value_or(3), config["ship"]["draft"].value_or(1), config["ship"]["height"].value_or(3),
		{config["ship"]["startPosition"]["X"].value_or(0.0),config["ship"]["startPosition"]["Y"].value_or(0.0)},
		config["ship"]["startAngle"].value_or(0.0), config["ship"]["startSpeed"].value_or(0.0));

Vector2D wind = {config["wind"]["X"].value_or(0.0), config["wind"]["Y"].value_or(0.0)}; 
Vector2D flow = {config["flow"]["X"].value_or(0.0), config["flow"]["Y"].value_or(0.0)};

double Thrust = 0.0;
double Angle = 0.0;

double timeScale = 0.2;
int i = 0;

//Вывод информации в консоль
void info(){
	std::cout << "Т=" << i*0.1 << "с | "
		<< "Поз: (" << boat.position.x << ", " << boat.position.y << ") | "
		<< "Штурвал: (" << Angle << ") | "
		<< "Тяга: (" << Thrust << ") | "
		<< "Курс: " << boat.angle * 180.0 / 3.14159265358979323846 << " град." << std::endl;
}

//то что выполняется вначале программы
void start(SDL_Window *window, SDL_Renderer *renderer){
	ship = IMG_LoadTexture(renderer, "Assets/ship.png");	
	SDL_SetRenderDrawColor(renderer, 99, 155, 255, 255);
}

//то что выполняется каждый кадр
void loop(SDL_Window *window, SDL_Renderer *renderer){
	//Считывание управления
	Thrust = fmin(fmax(Thrust - Engine_GetAxis::Y() * 0.1, -5.0),5);
	Angle = fmod(Angle - Engine_GetAxis::X(), 360.0);

	boat.update(0.1 * timeScale, Thrust, Angle, wind, flow);
	info();

    rect = {int(boat.position.x * 50), int(boat.position.y * 50), 180, 99};
	SDL_RenderCopyEx(renderer, ship, NULL, &rect, boat.angle  * 180.0 / 3.14159265358979323846, &center, SDL_FLIP_NONE);

	i++;
}
