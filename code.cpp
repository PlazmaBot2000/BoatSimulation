#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_FontCache.h>
#include "Boat.h"
#include "Engine/Engine_Modules.h"
#include <toml++/toml.hpp>
#include <ctime>
#include <format>



FC_Font* DepartureMonoNerdFont = FC_CreateFont(); 
FC_Font* DepartureMonoNerdFontTitle = FC_CreateFont(); 



struct IslandData {
    float drawX, drawY;
    float drawWidth, drawHeight;
    std::vector<Collider> colliders;
};

const double RAD_TO_DEG = 180.0 / 3.141592653589793;

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

SDL_Texture* flagTextures[8]; 
SDL_Rect flagRect;

SDL_Texture* flowArrowTexture;
SDL_Rect flowRect;
SDL_Point flowCenter = { 32, 32 };

auto config = toml::parse_file("Assets/Config/config.toml");

bool game_is_running = true;
bool pause = false;
int Prev_Tick = 0;

BoatSimulator boat(config["ship"]["mass"].value_or(50000), config["ship"]["length"].value_or(10),
		config["ship"]["beam"].value_or(3), config["ship"]["draft"].value_or(1), config["ship"]["height"].value_or(3),
		{config["ship"]["startPosition"]["X"].value_or(0.0),config["ship"]["startPosition"]["Y"].value_or(0.0)},
		config["ship"]["startAngle"].value_or(0.0), config["ship"]["startSpeed"].value_or(0.0));

Vector2D wind = {config["wind"]["X"].value_or(0.0), config["wind"]["Y"].value_or(0.0)}; 
Vector2D flow = {config["flow"]["X"].value_or(0.0), config["flow"]["Y"].value_or(0.0)};

std::vector<IslandData> Islands(config["IslandCount"].value_or(10));
int islandCount = config["IslandCount"].value_or(10);
bool Logs = config["Logs"].value_or(false);
Collider boatCollider;

double Thrust = 0.0;
double Angle = 0.0;

double timeScale = 0.2;
int i = 0;



std::string info() {
    return std::format(
        "Т={:.1f}с | Позиция: ({:.1f}, {:.1f}) | Штурвал: ({:.1f}) | Тяга: ({:.1f}) | Курс: {:.1f} град.",
        i * 0.1,
        boat.position.x, boat.position.y,
        Angle,
        Thrust,
        boat.angle * 180.0 / 3.14159265358979323846
    );
}



void loadFlagTextures(SDL_Renderer* renderer) {
    for (int i = 0; i < 8; i++) {
        std::string path = "Assets/Textures/flag_" + std::to_string(i) + ".png";
        flagTextures[i] = IMG_LoadTexture(renderer, path.c_str());
        if (!flagTextures[i]) {
            std::cerr << "Ошибка загрузки: " << path << " | " << IMG_GetError() << std::endl;
        }
    }
}


int GetWindDirectionIndex(Vector2D w) {
    if (std::abs(w.x) < 0.001 && std::abs(w.y) < 0.001) return 0;

    double angle = atan2(w.y, w.x); 

    double degrees = angle * 180.0 / 3.1415926535;
    if (degrees < 0) degrees += 360.0;

    return (int)((degrees + 22.5) / 45.0) % 8;
}


void GenIslands(){
	srand(time(NULL));
	
    Islands.clear(); 

	for (int i = 0; i < islandCount; i++) {
        Collider c1, c2, c3; // c1-нижний, c2-средний, c3-верхний

        float fullW = ISLAND_WIDTH * ISLAND_MULT;
        float fullH = ISLAND_HEIGHT * ISLAND_MULT;
        
        c1.width = fullW * 0.7f;       
        c1.height = fullH * 0.3f; 
        c1.angle = 0.0f;

        c2.width = fullW * 1.0f; 
        c2.height = fullH * 0.3f;
        c2.angle = 0.0f;

        c3.width = fullW * 0.7f; 
        c3.height = fullH * 0.4f;
        c3.angle = 0.0f;
    
		bool placed = false;
    	for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        	int nx = Engine_Random(MARGIN, 1920 - MARGIN - (int)fullW);
        	int ny = Engine_Random(MARGIN, 1080 - MARGIN - (int)fullH);
        
        	c3.x = nx + (fullW - c3.width) / 2.0f;
        	c3.y = ny + 20; 
            
            c2.x = nx + (fullW - c2.width) / 2.0f; 
            c2.y = ny + 20 + c3.height * 0.5f;

            c1.x = nx + (fullW - c1.width) / 2.0f; 
            c1.y = c2.y + c2.height * 0.8f;


        	bool hasCollision = false;
			if(c1.checkCollision(boatCollider) or c2.checkCollision(boatCollider) or c3.checkCollision(boatCollider)){
				hasCollision = true;
			}else{
        		for (int k = 0; k < Islands.size(); k++) {
                    for(const auto& existingCollider : Islands[k].colliders) {
            		    if (c2.checkCollision(existingCollider)) { 
                		    hasCollision = true;
                		    break;
					    }
                    }
                    if (hasCollision) break;
        		}
			}

        	if (!hasCollision) {
            	placed = true;
                IslandData currentIsland;
                currentIsland.drawX = nx;
                currentIsland.drawY = ny;
                currentIsland.drawWidth = fullW;
                currentIsland.drawHeight = fullH;

                currentIsland.colliders.push_back(c1);
                currentIsland.colliders.push_back(c2);
                currentIsland.colliders.push_back(c3);
                Islands.push_back(currentIsland); 
            	break;        	
            }
    	}

    	if (!placed) {
        	std::cerr << "Не удалось разместить остров #" << i << " (недостаточно места)" << std::endl;
    	}
	}
}


void Draw(SDL_Window *window, SDL_Renderer *renderer) { 
    for(int k = 0; k < Islands.size(); k++) {
        SDL_Rect dstRect = { 
            (int)Islands[k].drawX, 
            (int)Islands[k].drawY, 
            (int)Islands[k].drawWidth, 
            (int)Islands[k].drawHeight
        };
        SDL_RenderCopy(renderer, island, NULL, &dstRect);
    }

    rect = {int(boat.position.x * 100), int(boat.position.y * 100), 180, 99};
    const double RAD_TO_DEG = 180.0 / 3.141592653589793;
    SDL_RenderCopyEx(renderer, ship, NULL, &rect, boat.angle * RAD_TO_DEG, &center, SDL_FLIP_NONE);

    if(wind.x + wind.y != 0) SDL_RenderCopy(renderer, flagTextures[GetWindDirectionIndex(wind)], NULL, &flagRect);

	if(flow.x != 0 || flow.y != 0) {
    	double angleRad = atan2(flow.y, flow.x); 
    	double angleDeg = (angleRad * 180.0 / 3.14159265358979323846) + 90; 
    
    	SDL_RenderCopyEx(renderer, flowArrowTexture, NULL, &flowRect, angleDeg, &flowCenter, SDL_FLIP_NONE);
	}
}

void TableMenu(SDL_Window *window, SDL_Renderer *renderer, const char* Text) {
    Uint8 oldR, oldG, oldB, oldA;

    SDL_GetRenderDrawColor(renderer, &oldR, &oldG, &oldB, &oldA);

	float textHeight = FC_GetHeight(DepartureMonoNerdFontTitle, Text); 

    SDL_Rect rect;
    rect.x = WindowWidth / 3;
    rect.y = (WindowHeight - textHeight - 40) / 2;
    rect.w = WindowWidth / 3;
    rect.h = textHeight + 40;

	SDL_SetRenderDrawColor(renderer, 102, 57, 49, 255);
	SDL_RenderFillRect(renderer, &rect);

	rect.x += 20;
    rect.y += 20;
    rect.w -= 40;
    rect.h -= 40;

    SDL_SetRenderDrawColor(renderer, 143, 86, 59, 255); 
    SDL_RenderFillRect(renderer, &rect);

    int centerY = rect.y + (rect.h - (int)textHeight) / 2;

    SDL_Rect centeredRect = { rect.x, centerY, rect.w, (int)textHeight };
    
    FC_DrawBoxAlign(DepartureMonoNerdFontTitle, renderer, centeredRect, FC_ALIGN_CENTER, Text);

    SDL_SetRenderDrawColor(renderer, oldR, oldG, oldB, oldA);
}



int start(SDL_Window *window, SDL_Renderer *renderer){	
	SDL_GetWindowSize(window, &WindowWidth, &WindowHeight);

	FC_LoadFont(DepartureMonoNerdFont, renderer, "Assets/Fonts/DepartureMono/DepartureMonoNerdFont-Regular.ttf", 20, FC_MakeColor(255,255,255,255), TTF_STYLE_NORMAL);
	FC_LoadFont(DepartureMonoNerdFontTitle, renderer, "Assets/Fonts/DepartureMono/DepartureMonoNerdFont-Regular.ttf", 48, FC_MakeColor(255,255,255,255), TTF_STYLE_NORMAL);

	ship = IMG_LoadTexture(renderer, "Assets/Textures/ship.png");	
	island = IMG_LoadTexture(renderer, "Assets/Textures/Island.png");
	flowArrowTexture = IMG_LoadTexture(renderer, "Assets/Textures/Arrow.png");
	loadFlagTextures(renderer);

	flowRect = { WindowWidth - 174, WindowHeight - 74, 64, 64};
	if (!ship or !island or !flowArrowTexture) {
    	std::cerr << "Ошибка загрузки текстур: " << IMG_GetError() << std::endl;
	}
	SDL_SetRenderDrawColor(renderer, 99, 155, 255, 255);
	boatCollider.width = 180;
	boatCollider.height = 99;

	flagRect = { WindowWidth - 155, WindowHeight - 112, 115, 112 };

	GenIslands();
	return 0;
}


int loop(SDL_Window *window, SDL_Renderer *renderer){
	boatCollider.x = boat.position.x * 100;
	boatCollider.y = boat.position.y * 100;
	boatCollider.angle = boat.angle * RAD_TO_DEG;;

	Draw(window, renderer);

	for (int j = 0; j < Islands.size(); j++){ 
        for(const auto& islandCollider : Islands[j].colliders) {
			if(Logs) islandCollider.draw(renderer);
		    if(boatCollider.checkCollision(islandCollider)){
			    std::cout << "END.";
			    game_is_running = false;
		    }
        }
	}

	Thrust = fmin(fmax(Thrust - Engine_GetAxis::Y() * 0.1, -5.0),5);
	Angle = fmod(Angle - Engine_GetAxis::X() * ((Thrust > 0) ? 1.0 : ((Thrust < 0) ? -1.0 : 0.0)), 360.0);

	if(Logs) {
		std::cout << info() << std::endl;
		boatCollider.draw(renderer);
	}
	
	if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_P]) {
    	if (Engine_Delay(500, Prev_Tick)) {
			game_is_running = !game_is_running;
			pause = !pause;
		}
	}	

	if(game_is_running) {
		boat.update(0.1 * timeScale, Thrust, Angle, wind, flow);
	} else {
		if (!pause) {
			TableMenu(window, renderer, "Игра окончена.\nНажмите Esc чтобы выйти.");
		} else {
			TableMenu(window, renderer, "Пауза");
		}
	}
		
	i++;
	return 0;
}