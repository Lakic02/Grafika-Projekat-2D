#pragma once
#ifndef PERSON_MANAGER_H
#define PERSON_MANAGER_H

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm> 
#include <random>    
#include "SeatManager.h"
#include "Util.h"

struct Person {
    float x, y;
    float targetX, targetY;
    float startX, startY;
    float speed;
    bool reachedRow;
    bool seated;
    bool isExiting;
    bool hasLeft;
};

class PersonManager {
public:
    std::vector<Person> people;
    unsigned int personTexture;

    PersonManager() {
        personTexture = loadImageToTexture("person.png");
        if (personTexture == 0) {
            std::cout << "GRESKA: 'person.png' nije nadjen." << std::endl;
        }
    }

    void spawnPeople(const SeatManager& sm) {
        people.clear();
        std::vector<int> occupiedIndices;
        for (int i = 0; i < sm.seats.size(); i++) {
            if (sm.seats[i].state == RESERVED || sm.seats[i].state == SOLD) {
                occupiedIndices.push_back(i);
            }
        }

        if (occupiedIndices.empty()) return; // Niko ne ulazi

        int maxPeople = occupiedIndices.size();
        int minPeople = (maxPeople > 1) ? maxPeople / 2 : 1;
        int peopleCount = minPeople + rand() % (maxPeople - minPeople + 1);

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(occupiedIndices.begin(), occupiedIndices.end(), g);

        float startX = -0.98f;
        float startY = 0.6f;

        for (int i = 0; i < peopleCount; i++) {
            int seatIndex = occupiedIndices[i];
            const Seat& targetSeat = sm.seats[seatIndex];

            Person p;
            p.x = startX;
            p.y = startY;
            p.startX = startX;
            p.startY = startY;
            p.targetX = targetSeat.x;
            p.targetY = targetSeat.y;
            p.speed = 0.3f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.3f));
            p.reachedRow = false;
            p.seated = false;
            p.isExiting = false;
            p.hasLeft = false;

            people.push_back(p);
        }
    }

    void startExit() {
        for (Person& p : people) {
            p.isExiting = true;
            p.seated = false;
            p.reachedRow = false;
        }
    }

    void update(double deltaTime) {
        float dt = (float)deltaTime;

        for (Person& p : people) {
            if (p.hasLeft) continue;

            if (!p.isExiting) {
                if (p.seated) continue;

                if (!p.reachedRow) {
                    if (p.y > p.targetY) {
                        p.y -= p.speed * dt;
                        if (p.y <= p.targetY) {
                            p.y = p.targetY;
                            p.reachedRow = true;
                        }
                    }
                    else p.reachedRow = true;
                }
                else {
                    if (p.x < p.targetX) {
                        p.x += p.speed * dt;
                        if (p.x >= p.targetX) {
                            p.x = p.targetX;
                            p.seated = true;
                        }
                    }
                    else p.seated = true;
                }
            }
            else {
                if (p.x > p.startX) {
                    p.x -= p.speed * dt;
                    if (p.x <= p.startX) p.x = p.startX;
                }
                else {
                    if (p.y < p.startY) {
                        p.y += p.speed * dt;
                        if (p.y >= p.startY) {
                            p.y = p.startY;
                            p.hasLeft = true;
                        }
                    }
                    else p.hasLeft = true;
                }
            }
        }
    }

    // IZMENA: Ako nema ljudi, smatramo da su svi seli (da ne blokiramo logiku)
    bool areAllSeated() {
        if (people.empty()) return true;
        for (const auto& p : people) {
            if (!p.seated) return false;
        }
        return true;
    }

    bool areAllGone() {
        if (people.empty()) return true;
        for (const auto& p : people) {
            if (!p.hasLeft) return false;
        }
        return true;
    }

    void draw(unsigned int shaderProgram, int uPosLoc, int uSizeLoc, int uColorLoc, int uUseTextureLoc) {
        glUniform1i(uUseTextureLoc, 1);
        if (personTexture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, personTexture);
        }
        else {
            glUniform1i(uUseTextureLoc, 0);
        }

        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

        for (const Person& p : people) {
            if (p.hasLeft) continue;
            glUniform2f(uPosLoc, p.x, p.y);
            glUniform2f(uSizeLoc, 0.08f, 0.08f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    void clear() {
        people.clear();
    }
};

#endif