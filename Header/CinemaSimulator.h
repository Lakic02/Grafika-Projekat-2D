#pragma once
#ifndef CINEMA_SIMULATOR_H
#define CINEMA_SIMULATOR_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "PersonManager.h"
#include "SeatManager.h"
#include "Util.h" // Potrebno za loadImageToTexture

enum SimState {
    IDLE,
    ENTERING,
    MOVIE,
    EXITING
};

class CinemaSimulator {
public:
    SimState currentState;

    // Tajmeri i boje ekrana
    float movieTimer;
    const float MOVIE_DURATION = 20.0f;
    int frameCounter;
    float screenR, screenG, screenB;

    // --- NOVO: Teksture za vrata ---
    unsigned int texDoorOpen;
    unsigned int texDoorClose;

    CinemaSimulator() {
        // Ucitavanje tekstura vrata
        texDoorOpen = loadImageToTexture("open.png");
        texDoorClose = loadImageToTexture("close.png");

        if (texDoorOpen == 0 || texDoorClose == 0) {
            std::cout << "UPOZORENJE: Nedostaju slike 'open.png' ili 'close.png'!" << std::endl;
        }

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

            // Logika za praznu salu (Odmah film)
            if (pm.people.empty()) {
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
        // Kretanje ljudi
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

            // Treperenje ekrana
            if (frameCounter % 20 == 0) {
                screenR = (float)rand() / RAND_MAX;
                screenG = (float)rand() / RAND_MAX;
                screenB = (float)rand() / RAND_MAX;
            }

            if (movieTimer >= MOVIE_DURATION) {
                screenR = 0.9f; screenG = 0.9f; screenB = 0.9f;
                std::cout << "Film gotov." << std::endl;

                // Ako je sala prazna, odmah reset
                if (pm.people.empty()) {
                    std::cout << "Sala je bila prazna. Resetujem." << std::endl;
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
                for (auto& s : sm.seats) s.state = FREE;
                reset();
                std::cout << "Sala prazna. Reset sistema." << std::endl;
            }
            break;
        }
    }

    bool shouldDrawOverlay() {
        return currentState == IDLE;
    }

    // --- AZURIRANO CRTANJE VRATA ---
    void drawDoors(int uPosLoc, int uSizeLoc, int uColorLoc, int uUseTextureLoc) {
        // Obavezno ukljucujemo teksture
        glUniform1i(uUseTextureLoc, 1);
        glActiveTexture(GL_TEXTURE0);

        // Biramo teksturu na osnovu stanja
        if (currentState == ENTERING || currentState == EXITING) {
            // Vrata otvorena
            glBindTexture(GL_TEXTURE_2D, texDoorOpen);
        }
        else {
            // Vrata zatvorena (IDLE ili MOVIE)
            glBindTexture(GL_TEXTURE_2D, texDoorClose);
        }

        // Boja bela da bi tekstura imala svoje originalne boje
        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

        // Pozicija (Gornji levi ugao)
        glUniform2f(uPosLoc, -0.98f, 0.6f);
        glUniform2f(uSizeLoc, 0.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void drawScreen(int uPosLoc, int uSizeLoc, int uColorLoc, int uUseTextureLoc) {
        // Platno nema teksturu, samo boju
        glUniform1i(uUseTextureLoc, 0);
        glUniform4f(uColorLoc, screenR, screenG, screenB, 1.0f);
        glUniform2f(uPosLoc, -0.6f, 0.6f);
        glUniform2f(uSizeLoc, 1.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

#endif