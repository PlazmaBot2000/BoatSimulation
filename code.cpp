#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Boat.h"
#include "Engine/Engine_Modules.h"
#include <toml++/toml.hpp>
#include <ctime>



const int ISLAND_WIDTH = 62;
const int ISLAND_HEIGHT = 34;
const int ISLAND_MULT = 3;

const int MARGIN = 15;
const int MAX_ATTEMPTS = 500;
int WindowWidth, WindowHeight;

SDL_Texture* ship;
SDL_Texture* island;
SDL_Rect rect;
SDL_Point center = {180 / 2, 99 / 2};

auto config = toml::parse_file("config.toml");

BoatSimulator boat(config["ship"]["mass"].value_or(50000), config["ship"]["length"].value_or(10),
		config["ship"]["beam"].value_or(3), config["ship"]["draft"].value_or(1), config["ship"]["height"].value_or(3),
		{config["ship"]["startPosition"]["X"].value_or(0.0),config["ship"]["startPosition"]["Y"].value_or(0.0)},
		config["ship"]["startAngle"].value_or(0.0), config["ship"]["startSpeed"].value_or(0.0));

Vector2D wind = {config["wind"]["X"].value_or(0.0), config["wind"]["Y"].value_or(0.0)}; 
Vector2D flow = {config["flow"]["X"].value_or(0.0), config["flow"]["Y"].value_or(0.0)};

std::vector<Collider> Islands(config["IslandCount"].value_or(10));
int islandCount = config["IslandCount"].value_or(10);
bool Logs = config["Logs"].value_or(false);
Collider boatCollider;

double Thrust = 0.0;
double Angle = 0.0;

double timeScale = 0.2;
int i = 0;















//===================================================================================================================================================
//Вывод информации в консоль
void info(){
	std::cout << "Т=" << i*0.1 << "с | "
		<< "Поз: (" << boat.position.x << ", " << boat.position.y << ") | "
		<< "Штурвал: (" << Angle << ") | "
		<< "Тяга: (" << Thrust << ") | "
		<< "Курс: " << boat.angle * 180.0 / 3.14159265358979323846 << " град." << std::endl;
}

void GenIslands(){
	srand(time(NULL));
	

	for (int i = 0; i < islandCount; i++) {
    	Islands[i].width = ISLAND_WIDTH * ISLAND_MULT;
    	Islands[i].height = ISLAND_HEIGHT * ISLAND_MULT;
    
		bool placed = false;
    	for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        	int nx = Engine_Random(MARGIN, 1920 - MARGIN - ISLAND_WIDTH * ISLAND_MULT);
        	int ny = Engine_Random(MARGIN, 1080 - MARGIN - ISLAND_HEIGHT * ISLAND_MULT);
        
        	Islands[i].x = nx;
        	Islands[i].y = ny;

        	bool hasCollision = false;
			if(Islands[i].checkCollision(boatCollider)){
				hasCollision = true;
			}else{
        		for (int k = 0; k < i; k++) {
            		if (Islands[i].checkCollision(Islands[k])) {
                		hasCollision = true;
                		break;
					}
        		}
			}

        	if (!hasCollision) {
            	placed = true;
            	break;        	}
    	}

    	if (!placed) {
        	std::cerr << "Не удалось разместить остров #" << i << " (недостаточно места)" << std::endl;
    	}
	}
}

void Draw(SDL_Window *window, SDL_Renderer *renderer) { 
    // Рисуем острова
    int islandCount = config["IslandCount"].value_or(10);
    for(int k = 0; k < islandCount; k++) {
        SDL_Rect dstRect = { 
            (int)Islands[k].x, 
            (int)Islands[k].y, 
            (int)Islands[k].width, 
            (int)Islands[k].height
        };
        SDL_RenderCopy(renderer, island, NULL, &dstRect);
    }

	// Рисуем корабль
    rect = {int(boat.position.x * 100), int(boat.position.y * 100), 180, 99};
    const double RAD_TO_DEG = 180.0 / 3.141592653589793;
    SDL_RenderCopyEx(renderer, ship, NULL, &rect, boat.angle * RAD_TO_DEG, &center, SDL_FLIP_NONE);
}
//===================================================================================================================================================















//то что выполняется вначале программы
void start(SDL_Window *window, SDL_Renderer *renderer){	
	SDL_GetWindowSize(window, &WindowWidth, &WindowHeight);
	ship = IMG_LoadTexture(renderer, "Assets/ship.png");	
	island = IMG_LoadTexture(renderer, "Assets/Island.png");
	if (!ship or !island) {
    	std::cerr << "Ошибка загрузки текстур: " << IMG_GetError() << std::endl;
	}
	SDL_SetRenderDrawColor(renderer, 99, 155, 255, 255);
	boatCollider.width = 180;
	boatCollider.height = 99;

	GenIslands();
}

//то что выполняется каждый кадр
void loop(SDL_Window *window, SDL_Renderer *renderer){
	boatCollider.x = boat.position.x * 100;
	boatCollider.y = boat.position.y * 100;
	boatCollider.angle = boat.angle;
	for (int j = 0; j < islandCount; j++){
		if(boatCollider.checkCollision(Islands[j])){
			std::cout << "THATS ALL";
			SDL_DestroyRenderer(renderer);
    		SDL_DestroyWindow(window);
		}
	}

	//Считывание управления
	Thrust = fmin(fmax(Thrust - Engine_GetAxis::Y() * 0.1, -5.0),5);
	Angle = fmod(Angle - Engine_GetAxis::X() * ((Thrust > 0) ? 1.0 : ((Thrust < 0) ? -1.0 : 0.0)), 360.0);

	boat.update(0.1 * timeScale, Thrust, Angle, wind, flow);
	
	if(Logs) info();
	Draw(window, renderer);
	boatCollider.draw(renderer);
	i++;
}
