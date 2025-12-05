#pragma once
#ifndef CINEMA_SIMULATOR_H
#define CINEMA_SIMULATOR_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "PersonManager.h"
#include "SeatManager.h"

enum SimState {
    IDLE,       // Rezervacija, overlay prisutan, vrata zatvorena
    ENTERING,   // Ljudi ulaze, vrata otvorena
    MOVIE,      // Svi seli, vrata zatvorena, platno treperi
    EXITING     // Film gotov, vrata otvorena, ljudi izlaze
};

class CinemaSimulator {
public:
    SimState currentState;

    // Za film
    float movieTimer;
    const float MOVIE_DURATION = 20.0f; // 20 sekundi

    // Za treperenje ekrana
    int frameCounter;
    float screenR, screenG, screenB;

    CinemaSimulator() {
        reset();
    }

    void reset() {
        currentState = IDLE;
        movieTimer = 0.0f;
        frameCounter = 0;
        screenR = 0.9f; screenG = 0.9f; screenB = 0.9f; // Pocetna bela boja
    }

    // Poziva se na ENTER
    void startProjection(PersonManager& pm, SeatManager& sm) {
        if (currentState == IDLE) {
            pm.spawnPeople(sm);
            currentState = ENTERING;
            std::cout << "Pocinje projekcija! Ljudi ulaze..." << std::endl;
        }
    }

    void update(double deltaTime, PersonManager& pm, SeatManager& sm) {
        // Update kretanja ljudi (bilo ulazak ili izlazak)
        if (currentState == ENTERING || currentState == EXITING) {
            pm.update(deltaTime);
        }

        // LOGIKA STANJA
        switch (currentState) {

        case ENTERING:
            // Provera da li su svi seli
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

            // Menjaj boju platna na svakih 20 frejmova
            if (frameCounter % 20 == 0) {
                screenR = (float)rand() / RAND_MAX;
                screenG = (float)rand() / RAND_MAX;
                screenB = (float)rand() / RAND_MAX;
            }

            // Ako prodje 20 sekundi
            if (movieTimer >= MOVIE_DURATION) {
                currentState = EXITING;
                pm.startExit(); // Naredi ljudima da izadju
                screenR = 0.9f; screenG = 0.9f; screenB = 0.9f; // Platno opet belo
                std::cout << "Film gotov. Izlazak!" << std::endl;
            }
            break;

        case EXITING:
            // Provera da li su svi izasli
            if (pm.areAllGone()) {
                // Svi izasli -> RESETUJ SVE
                pm.clear(); // Brisi ljude

                // Resetuj sedista (Sva postaju FREE)
                for (auto& s : sm.seats) {
                    s.state = FREE;
                }

                reset(); // Vraca na IDLE
                std::cout << "Sala prazna. Reset sistema." << std::endl;
            }
            break;
        }
    }

    // --- FUNKCIJE ZA CRTANJE ---

    // Vraca true ako overlay (tamna zavesa) treba da bude ukljucen
    bool shouldDrawOverlay() {
        return currentState == IDLE;
    }

    // Crta vrata sa odgovarajucom bojom
    void drawDoors(int uPosLoc, int uSizeLoc, int uColorLoc) {
        // Svetlo plava ako su otvorena (ENTERING ili EXITING), Tamno plava inace
        if (currentState == ENTERING || currentState == EXITING) {
            glUniform4f(uColorLoc, 0.0f, 1.0f, 1.0f, 1.0f); // Cyan (Otvoreno)
        }
        else {
            glUniform4f(uColorLoc, 0.1f, 0.1f, 0.6f, 1.0f); // Tamno plava (Zatvoreno)
        }

        glUniform2f(uPosLoc, -0.98f, 0.6f);
        glUniform2f(uSizeLoc, 0.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Crta platno sa odgovarajucom bojom
    void drawScreen(int uPosLoc, int uSizeLoc, int uColorLoc, int uUseTextureLoc) {
        glUniform1i(uUseTextureLoc, 0);

        // Koristimo izracunatu boju (bela ili random tokom filma)
        glUniform4f(uColorLoc, screenR, screenG, screenB, 1.0f);

        glUniform2f(uPosLoc, -0.6f, 0.6f);
        glUniform2f(uSizeLoc, 1.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

#endif