#include <iostream>
#include <cmath>
#include <algorithm>
#include "Engine/Engine_Modules.h"

#ifndef BOAT_H
#define BOAT_H 


class BoatSimulator {
public:
    double mass, length, beam, draft, height;
    Vector2D position;
    Vector2D velocity;
    double angle;           //угол судна в радианах
    double angularVelocity;
    
    double virtualMassX, virtualMassY, virtualInertia;
    const double waterDensity = 1025.0;
    const double airDensity = 1.225;

    BoatSimulator(double m, double L, double B, double T, double H, Vector2D P, double startAngleDegrees = 0.0, double startSpeed = 0.0) 
        : mass(m), length(L), beam(B), draft(T), height(H), position(P) {
        
        //начальный угол в радианы
        angle = startAngleDegrees * 3.14159265358979323846 / 180.0;
        
        velocity.x = std::cos(angle) * startSpeed;
        velocity.y = std::sin(angle) * startSpeed;
        
        position = {0.0, 0.0};
        angularVelocity = 0.0;

        virtualMassX = mass * 1.1;
        virtualMassY = mass * 1.8;
        double I = (mass * (length * length + beam * beam)) / 12.0;
        virtualInertia = I * 1.4;
    }

    void update(double deltaTime, double throttle, double rudderAngle, Vector2D windVel, Vector2D flowVel) {
        Vector2D forward = { std::cos(angle), std::sin(angle) };
        Vector2D right = { std::sin(angle), -std::cos(angle) };

        //тяга двигателя по курсу
        double maxThrust = mass * 0.02; 
        Vector2D thrustForce = forward * (throttle * maxThrust);

        //относительные скорости
        Vector2D relWaterVel = velocity - flowVel;
        Vector2D relWindVel = windVel - velocity;

        //сопротивление
        Vector2D dragForce = calculateWaterDrag(relWaterVel, forward, right);
        Vector2D windForce = calculateWindForce(relWindVel, forward, right);

        //вращение
    	double speed = relWaterVel.magnitude();

		double vWindSide = relWindVel.x * right.x + relWindVel.y * right.y;
		double windTorque = vWindSide * std::abs(vWindSide) * airDensity * (length * height) * 0.1;

		double rudderTorque = -rudderAngle * speed * (mass * 0.01) * length;
		double dampingTorque = -angularVelocity * virtualInertia * (7.0 + speed);

		double angularAcc = (rudderTorque + dampingTorque + windTorque) / virtualInertia;
		angularVelocity += angularAcc * deltaTime;
		angle += angularVelocity * deltaTime;

        //движение
        Vector2D totalForce = thrustForce + dragForce + windForce;
        
		double forceFwdLocal = totalForce.x * forward.x + totalForce.y * forward.y;
		double forceSideLocal = totalForce.x * right.x + totalForce.y * right.y;

		double accFwd = forceFwdLocal / virtualMassX;
		double accSide = forceSideLocal / virtualMassY;

		Vector2D acceleration = (forward * accFwd) + (right * accSide);

        velocity = velocity + acceleration * deltaTime;
        position = position + velocity * deltaTime;
		velocity = velocity * (1.0 - 0.1 * deltaTime); 
    }

private:
    Vector2D calculateWaterDrag(Vector2D relVel, Vector2D forward, Vector2D right) {
    double vFwd = relVel.x * forward.x + relVel.y * forward.y;
    double vSide = relVel.x * right.x + relVel.y * right.y;

    double forceFwd = -(0.5 * waterDensity * (beam * draft) * 1.0 * vFwd * std::abs(vFwd) + 100.0 * vFwd);
    double forceSide = -(0.5 * waterDensity * (length * draft) * 2.5 * vSide * std::abs(vSide) + 2000.0 * vSide);

    return (forward * forceFwd) + (right * forceSide);
}
    Vector2D calculateWindForce(Vector2D relWind, Vector2D forward, Vector2D right) {
        double vFwd = relWind.x * forward.x + relWind.y * forward.y;
        double vSide = relWind.x * right.x + relWind.y * right.y;

        double forceFwd = 0.5 * airDensity * (beam * (height - draft)) * 1.5 * vFwd * std::abs(vFwd);
        double forceSide = 0.5 * airDensity * (length * (height - draft)) * 1.5 * vSide * std::abs(vSide);

        return (forward * forceFwd) + (right * forceSide);
    }
};

#endif
