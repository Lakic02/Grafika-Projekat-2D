#pragma once
#ifndef PERSON_MANAGER_H
#define PERSON_MANAGER_H

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm> // Za random shuffle
#include <random>    // Za random generator
#include "../Header/Util.h"
#include "../Header/SeatManager.h"

// Struktura koja definise jednu osobu
struct Person {
    float x, y;             // Trenutna pozicija
    float targetX, targetY; // Ciljna pozicija (sediste)
    float speed;            // Brzina kretanja
    bool reachedRow;        // Da li je stigao do odgovarajuceg reda (Y osa)
    bool seated;            // Da li je stigao na sediste
};

class PersonManager {
public:
    std::vector<Person> people;
    unsigned int personTexture; // ID teksture coveculjka

    PersonManager() {
        // Ucitavamo sliku coveculjka. 
        // NAPOMENA: Potrebno je imati "person.png" u folderu projekta!
        personTexture = loadImageToTexture("person.png");
        if (personTexture == 0) {
            std::cout << "GRESKA: 'person.png' nije nadjen. Koristice se boja umesto slike." << std::endl;
        }
    }

    // Poziva se KAD SE PRITISNE ENTER
    void spawnPeople(const SeatManager& sm) {
        people.clear();

        // 1. Pronadji sva zauzeta mesta (RESERVED ili SOLD)
        std::vector<int> occupiedIndices;
        for (int i = 0; i < sm.seats.size(); i++) {
            if (sm.seats[i].state == RESERVED || sm.seats[i].state == SOLD) {
                occupiedIndices.push_back(i);
            }
        }

        if (occupiedIndices.empty()) return; // Niko nije kupio kartu, niko ne ulazi

        // 2. Odredimo koliko ljudi ulazi (Random broj)
        // Recimo: minimum polovina zauzetih, maksimum svi zauzeti.
        int maxPeople = occupiedIndices.size();
        int minPeople = (maxPeople > 1) ? maxPeople / 2 : 1;

        int peopleCount = minPeople + rand() % (maxPeople - minPeople + 1);

        std::cout << "Ukupno sedista zauzeto: " << maxPeople << ". Ulazi ljudi: " << peopleCount << std::endl;

        // 3. Promesamo indekse da bi ljudi isli na nasumicna zauzeta mesta
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(occupiedIndices.begin(), occupiedIndices.end(), g);

        // 4. Kreiramo ljude
        // Vrata su na (-0.98, 0.6) - Gornji levi ugao
        float startX = -0.98f;
        float startY = 0.6f;

        for (int i = 0; i < peopleCount; i++) {
            int seatIndex = occupiedIndices[i];
            const Seat& targetSeat = sm.seats[seatIndex];

            Person p;
            p.x = startX;
            p.y = startY;
            // Ciljamo centar sedista (ili donji levi ugao sedista, zavisno kako zelimo)
            p.targetX = targetSeat.x;
            p.targetY = targetSeat.y;

            // Random brzina da ne idu svi kao roboti isto
            // Brzina izmedju 0.3 i 0.6
            p.speed = 0.3f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.3f));

            p.reachedRow = false;
            p.seated = false;

            people.push_back(p);
        }
    }

    // Logika kretanja: Prvo Y (do reda), pa onda X (do sedista)
    void update(double deltaTime) {
        for (Person& p : people) {
            if (p.seated) continue;

            float dt = (float)deltaTime;

            // Faza 1: Kretanje vertikalno (po Y osi) nadole
            if (!p.reachedRow) {
                if (p.y > p.targetY) {
                    p.y -= p.speed * dt;
                    // Ako smo preskocili cilj, vratimo ga
                    if (p.y <= p.targetY) {
                        p.y = p.targetY;
                        p.reachedRow = true;
                    }
                }
                else {
                    // Ako je vec ispod ili jednako (teoretski nemoguce jer krecu od gore)
                    p.reachedRow = true;
                }
            }
            // Faza 2: Kretanje horizontalno (po X osi) udesno
            else {
                if (p.x < p.targetX) {
                    p.x += p.speed * dt;
                    if (p.x >= p.targetX) {
                        p.x = p.targetX;
                        p.seated = true;
                    }
                }
                else {
                    // Za svaki slucaj
                    p.seated = true;
                }
            }
        }
    }

    void draw(unsigned int shaderProgram, int uPosLoc, int uSizeLoc, int uColorLoc, int uUseTextureLoc) {
        // Omoguci teksture
        glUniform1i(uUseTextureLoc, 1);
        if (personTexture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, personTexture);
        }
        else {
            glUniform1i(uUseTextureLoc, 0); // Ako nema slike, crtaj kvadrat bojom
        }

        // Boja coveculjka (bela da bi se videla originalna tekstura)
        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

        for (const Person& p : people) {
            // Velicina coveculjka malo manja od sedista
            glUniform2f(uPosLoc, p.x, p.y);
            glUniform2f(uSizeLoc, 0.08f, 0.08f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
};

#endif