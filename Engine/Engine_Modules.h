#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include <algorithm>

#ifndef MODULES_H
#define MODULES_H

class Engine_GetAxis{
	public:
		static int X();
		static int Y();
};

struct Vector2D {
    double x, y;
    Vector2D operator+(const Vector2D& other) const { return {x + other.x, y + other.y}; }
    Vector2D operator-(const Vector2D& other) const { return {x - other.x, y - other.y}; }
    Vector2D operator*(double scalar) const { return {x * scalar, y * scalar}; }
    double magnitude() const { return std::sqrt(x * x + y * y); }
};

class Collider {
public:
    float x, y;          // Позиция 
	float width, height; // Размеры
    float angle = 0.0f;  // Угол поворота в радианах
    
    bool checkCollision(const Collider& b) const {
        if (angle == 0.0f && b.angle == 0.0f) {
            return (x < b.x + b.width &&
                    x + width > b.x &&
                    y < b.y + b.height &&
                    y + height > b.y);
        }
        return checkCollisionSAT(b);
    }

    void draw(SDL_Renderer* renderer) const {
        auto vertices = getVertices();
		Uint8 r, g, b, a;
		
		SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

        for (int i = 0; i < 4; i++) {
            const Vector2D& p1 = vertices[i];
            const Vector2D& p2 = vertices[(i + 1) % 4];

            SDL_RenderDrawLine(renderer, 
                static_cast<int>(p1.x), static_cast<int>(p1.y), 
                static_cast<int>(p2.x), static_cast<int>(p2.y));
        }
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }
private:
    std::vector<Vector2D> getVertices() const {
        float cx = x + width / 2.0f;
        float cy = y + height / 2.0f;
        float cosA = cosf(angle);
        float sinA = sinf(angle);

        std::vector<Vector2D> vertices(4);
        float dx[4] = {-width/2, width/2, width/2, -width/2};
        float dy[4] = {-height/2, -height/2, height/2, height/2};

        for (int i = 0; i < 4; i++) {
            vertices[i].x = cx + dx[i] * cosA - dy[i] * sinA;
            vertices[i].y = cy + dx[i] * sinA + dy[i] * cosA;
        }
        return vertices;
    }

    bool checkCollisionSAT(const Collider& b) const {
        auto v1 = getVertices();
        auto v2 = b.getVertices();

        auto checkAxis = [&](const std::vector<Vector2D>& r1, const std::vector<Vector2D>& r2) {
            for (int i = 0; i < 4; i++) {
                Vector2D p1 = r1[i];
                Vector2D p2 = r1[(i + 1) % 4];
                Vector2D axis = {-(p2.y - p1.y), p2.x - p1.x};

                auto getMinMax = [&](const std::vector<Vector2D>& r, float& min, float& max) {
                    min = max = (r[0].x * axis.x + r[0].y * axis.y);
                    for (const auto& p : r) {
                        float proj = (p.x * axis.x + p.y * axis.y);
                        if (proj < min) min = proj;
                        if (proj > max) max = proj;
                    }
                };

                float min1, max1, min2, max2;
                getMinMax(r1, min1, max1);
                getMinMax(r2, min2, max2);

                if (max1 < min2 || max2 < min1) return false;
            }
            return true;
        };

        return checkAxis(v1, v2) && checkAxis(v2, v1);
    }
};


bool Engine_Delay(int time, int& Prev_Tick);
int Engine_Random(int upper, int lower);
#endif
