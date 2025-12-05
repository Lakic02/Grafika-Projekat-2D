#pragma once
#ifndef SEAT_MANAGER_H
#define SEAT_MANAGER_H

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Struktura koja predstavlja jedno sediste
struct Seat {
    float x;      // NDC X pozicija (donji levi ugao)
    float y;      // NDC Y pozicija (donji levi ugao)
    float width;  // NDC sirina
    float height; // NDC visina
    bool isReserved; // false = slobodno (plavo), true = rezervisano (zuto)
};

class SeatManager {
public:
    std::vector<Seat> seats;
    bool oldLeftClickState; // Pamtimo da li je taster bio pritisnut u proslom frejmu

    SeatManager() {
        oldLeftClickState = false;
        initSeats();
    }

    // Funkcija koja popunjava vektor sedista (tvoja stara logika iz main-a, sada ovde)
    void initSeats() {
        float startX = -0.5f;
        float startY = 0.3f;
        float gapX = 0.13f;
        float gapY = 0.15f;
        float seatW = 0.1f;
        float seatH = 0.1f;

        for (int i = 0; i < 8; i++) // 8 Redova
        {
            for (int j = 0; j < 8; j++) // 8 Kolona
            {
                Seat s;
                s.x = startX + j * gapX;
                s.y = startY - i * gapY;
                s.width = seatW;
                s.height = seatH;
                s.isReserved = false; // Na pocetku su sva slobodna
                seats.push_back(s);
            }
        }
    }

    // Funkcija koja obradjuje klikove
    void processInput(GLFWwindow* window, int screenWidth, int screenHeight) {

        // Uzimamo trenutno stanje levog klika
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        // DETEKCIJA KLIKA (Rising Edge):
        // Reagujemo samo ako je sada pritisnut (PRESS), a ranije nije bio (RELEASE)
        // Ovo sprecava da se sediste pali/gasi 60 puta u sekundi dok drzimo taster
        if (state == GLFW_PRESS && oldLeftClickState == GLFW_RELEASE)
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // KONVERZIJA: Iz Screen Space (Pikseli) u NDC (-1 do 1)
            // Formula: NDC_X = (2 * x / w) - 1
            // Formula: NDC_Y = 1 - (2 * y / h)  <-- Y osa je obrnuta u OpenGL-u!

            float ndcX = (2.0f * (float)mouseX / (float)screenWidth) - 1.0f;
            float ndcY = 1.0f - (2.0f * (float)mouseY / (float)screenHeight);

            // Prolazimo kroz sva sedista i proveravamo da li je kliknuto na neko
            for (Seat& s : seats) {
                // AABB Collision Detection (Tacka u pravougaoniku)
                // Da li je mis desno od leve ivice I levo od desne ivice... itd.
                if (ndcX >= s.x && ndcX <= (s.x + s.width) &&
                    ndcY >= s.y && ndcY <= (s.y + s.height))
                {
                    // Kliknuto je sediste! Obrni status.
                    s.isReserved = !s.isReserved;
                    // Prekidamo petlju jer ne mozemo kliknuti dva sedista odjednom
                    break;
                }
            }
        }

        // Azuriramo staro stanje za sledeci frejm
        oldLeftClickState = (state == GLFW_PRESS);
    }
};

#endif