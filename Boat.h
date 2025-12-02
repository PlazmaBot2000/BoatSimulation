#include <bits/stdc++.h>
#ifndef BOAT_H
#define BOAT_H

struct Vector2D {
    double x, y;

    Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }
    Vector2D operator-(const Vector2D& other) const {
        return {x - other.x, y - other.y};
    }
    Vector2D operator*(double scalar) const {
        return {x * scalar, y * scalar};
    }
    Vector2D operator/(double scalar) const {
        return {x / scalar, y / scalar};
    }
    double magnitude() const {
        return std::sqrt(x * x + y * y);
    }
};

class BoatSimulator {
public:
    double mass;          // масса (кг)
    double length;        // длина (м)
    double beam;          // ширина (м)
    double height = 2.0; // высота (м)
    double draft;         // осадка (м)

    //плотности
    const double waterDensity = 1025.0; // кг/м^3
    const double airDensity = 1.225;   // кг/м^3

    Vector2D position;
    Vector2D velocity;

    double virtualMassX;
    double virtualMassY;

    BoatSimulator(double m, double L, double B, double T, double H) 
        : mass(m), length(L), beam(B), draft(T), height(H) {
        virtualMassX = mass * (1.0 + 0.1); 
        virtualMassY = mass * (1.0 + 0.8); 

        position = {0.0, 0.0};
        velocity = {0.0, 0.0};
    }

    void update(double deltaTime, Vector2D windVel, Vector2D currentVel, Vector2D thrustForce) {
        
        Vector2D relativeWaterVelocity = velocity - currentVel; 
        Vector2D relativeWindVelocity = windVel - velocity;

        Vector2D dragForce = calculateWaterDrag(relativeWaterVelocity);
        Vector2D windForce = calculateWindForce(relativeWindVelocity);

        Vector2D totalForce = thrustForce + dragForce + windForce;

        Vector2D acceleration;
        acceleration.x = totalForce.x / virtualMassX;
        acceleration.y = totalForce.y / virtualMassY;

        velocity = velocity + acceleration * deltaTime;
        position = position + velocity * deltaTime;

    }

private:
    Vector2D calculateWaterDrag(Vector2D relVel) {
		//коэфициенты сопротивления(примерные)
        double Cd_x = 1.2; 
        double Cd_y = 1.8;
        
        double areaX = beam * draft;
        double areaY = length * draft;

        Vector2D drag;
        drag.x = -0.5 * waterDensity * areaX * Cd_x * relVel.x * std::abs(relVel.x);
        drag.y = -0.5 * waterDensity * areaY * Cd_y * relVel.y * std::abs(relVel.y);
        
        return drag;
    }

    Vector2D calculateWindForce(Vector2D relWindVel) {
		//коэфициент сопротивления воздуха (опять же примерный)
        double Cd_air = 1.5; 
        
        double areaWindX = beam * (height - draft);
        double areaWindY = length * (height - draft);

        Vector2D windF;
        windF.x = -0.5 * airDensity * areaWindX * Cd_air * relWindVel.x * std::abs(relWindVel.x);
        windF.y = -0.5 * airDensity * areaWindY * Cd_air * relWindVel.y * std::abs(relWindVel.y);

        return windF;
    }
};

#endif
