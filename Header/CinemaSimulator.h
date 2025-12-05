#pragma once
#ifndef CINEMA_SIMULATOR_H
#define CINEMA_SIMULATOR_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "PersonManager.h"
#include "SeatManager.h"

enum SimState {
    IDLE,
    ENTERING,
    MOVIE,
    EXITING
};

class CinemaSimulator {
public:
    SimState currentState;
    float movieTimer;
    const float MOVIE_DURATION = 20.0f;

    int frameCounter;
    float screenR, screenG, screenB;

    CinemaSimulator() {
        reset();
    }

    void reset() {
        currentState = IDLE;
        movieTimer = 0.0f;
        frameCounter = 0;
        screenR = 0.9f; screenG = 0.9f; screenB = 0.9f;
    }

    void startProjection(PersonManager& pm, SeatManager& sm) {
        if (currentState == IDLE) {
            pm.spawnPeople(sm);

            // LOGIKA ZA PRAZNU SALU (Ulazak)
            if (pm.people.empty()) {
                // Ako nema ljudi, ODMAH prelazimo na film
                std::cout << "Sala prazna! Preskacemo ulazak, film odmah pocinje." << std::endl;
                currentState = MOVIE;
                movieTimer = 0.0f;
                frameCounter = 0;
            }
            else {
                currentState = ENTERING;
                std::cout << "Pocinje projekcija! Ljudi ulaze..." << std::endl;
            }
        }
    }

    void update(double deltaTime, PersonManager& pm, SeatManager& sm) {
        if (currentState == ENTERING || currentState == EXITING) {
            pm.update(deltaTime);
        }

        switch (currentState) {

        case ENTERING:
            if (pm.areAllSeated()) {
                currentState = MOVIE;
                movieTimer = 0.0f;
                frameCounter = 0;
                std::cout << "Svi su seli. Film pocinje! Vrata se zatvaraju." << std::endl;
            }
            break;

        case MOVIE:
            movieTimer += (float)deltaTime;
            frameCounter++;

            if (frameCounter % 20 == 0) {
                screenR = (float)rand() / RAND_MAX;
                screenG = (float)rand() / RAND_MAX;
                screenB = (float)rand() / RAND_MAX;
            }

            if (movieTimer >= MOVIE_DURATION) {
                screenR = 0.9f; screenG = 0.9f; screenB = 0.9f;
                std::cout << "Film gotov." << std::endl;

                // LOGIKA ZA PRAZNU SALU (Izlazak)
                if (pm.people.empty()) {
                    // Ako nema ljudi, ODMAH resetuj sve
                    std::cout << "Sala je bila prazna. Odmah resetujem." << std::endl;

                    // Moramo rucno ocistiti status sedista (iako niko nije sedeo, cisto radi konzistentnosti)
                    for (auto& s : sm.seats) s.state = FREE;

                    reset();
                }
                else {
                    currentState = EXITING;
                    pm.startExit();
                    std::cout << "Gosti izlaze..." << std::endl;
                }
            }
            break;

        case EXITING:
            if (pm.areAllGone()) {
                pm.clear();
                for (auto& s : sm.seats) {
                    s.state = FREE;
                }
                reset();
                std::cout << "Sala prazna. Reset sistema." << std::endl;
            }
            break;
        }
    }

    bool shouldDrawOverlay() {
        return currentState == IDLE;
    }

    void drawDoors(int uPosLoc, int uSizeLoc, int uColorLoc) {
        if (currentState == ENTERING || currentState == EXITING) {
            glUniform4f(uColorLoc, 0.0f, 1.0f, 1.0f, 1.0f);
        }
        else {
            glUniform4f(uColorLoc, 0.1f, 0.1f, 0.6f, 1.0f);
        }

        glUniform2f(uPosLoc, -0.98f, 0.6f);
        glUniform2f(uSizeLoc, 0.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void drawScreen(int uPosLoc, int uSizeLoc, int uColorLoc, int uUseTextureLoc) {
        glUniform1i(uUseTextureLoc, 0);
        glUniform4f(uColorLoc, screenR, screenG, screenB, 1.0f);
        glUniform2f(uPosLoc, -0.6f, 0.6f);
        glUniform2f(uSizeLoc, 1.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

#endif