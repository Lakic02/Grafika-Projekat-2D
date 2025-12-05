#pragma once
#ifndef SEAT_MANAGER_H
#define SEAT_MANAGER_H

#include <vector>
#include <iostream> 
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Definisemo moguca stanja sedista
enum SeatState {
    FREE,       // Slobodno (Plavo)
    RESERVED,   // Rezervisano (Zuto)
    SOLD        // Kupljeno (Crveno)
};

// Struktura koja predstavlja jedno sediste
struct Seat {
    float x;        // NDC X pozicija (donji levi ugao)
    float y;        // NDC Y pozicija (donji levi ugao)
    float width;    // NDC sirina
    float height;   // NDC visina
    SeatState state; // <-- Promenjeno iz bool u enum
};

class SeatManager {
public:
    std::vector<Seat> seats;

    // Za detekciju "Rising Edge" (jedan klik)
    bool oldLeftClickState;
    bool oldKeyStates[10]; // Za tastere 0-9 (iako koristimo 1-9)

    SeatManager() {
        oldLeftClickState = false;
        // Inicijalizujemo stare state-ove tastera na false
        for (int i = 0; i < 10; i++) oldKeyStates[i] = false;

        initSeats();
    }

    // Funkcija koja popunjava vektor sedista
    void initSeats() {
        float startX = -0.5f;
        float startY = 0.3f;
        float gapX = 0.13f;
        float gapY = 0.15f;
        float seatW = 0.1f;
        float seatH = 0.1f;

        for (int i = 0; i < 8; i++) // 8 Redova (Indeks i: 0=Gore, 7=Dole)
        {
            for (int j = 0; j < 8; j++) // 8 Kolona (Indeks j: 0=Levo, 7=Desno)
            {
                Seat s;
                s.x = startX + j * gapX;
                s.y = startY - i * gapY;
                s.width = seatW;
                s.height = seatH;
                s.state = FREE; // Na pocetku su sva slobodna
                seats.push_back(s);
            }
        }
    }

    // --- LOGIKA ZA KUPOVINU KARATA (NOVI TASK) ---
    void buyTickets(int n) {
        if (n <= 0 || n > 8) return; // Zastita: ne mozemo kupiti vise od 8 u redu

        // Trazimo od poslednjeg reda (red 7) ka prvom (red 0)
        // I unutar reda od krajnjeg desnog (kolona 7) ka levom

        for (int row = 7; row >= 0; --row) {
            // Unutar reda trazimo N susednih.
            // Posto trazimo od najdesnijeg, krecemo 'startCol' od 7.
            // Moramo imati prostora za N sedista levo od startCol.
            // Primer: N=2. Mozemo proveriti (7,6), (6,5)... poslednja opcija je (1,0).

            for (int col = 7; col >= n - 1; --col) {
                bool foundGroup = true;

                // Proveravamo grupu od N sedista pocevsi od 'col' pa na levo
                for (int k = 0; k < n; ++k) {
                    int checkCol = col - k;
                    int index = row * 8 + checkCol; // Formula za 1D niz iz 2D petlje

                    if (seats[index].state != FREE) {
                        foundGroup = false;
                        break; // Prekini provera ove grupe, naleteli smo na zauzeto
                    }
                }

                // Ako smo nasli grupu
                if (foundGroup) {
                    std::cout << "Kupovina uspesna! Red: " << row + 1 << ", " << n << " sedista." << std::endl;
                    // Markiramo ih kao SOLD (Crveno)
                    for (int k = 0; k < n; ++k) {
                        int index = row * 8 + (col - k);
                        seats[index].state = SOLD;
                    }
                    return; // Zavrsavamo funkciju cim kupimo prvu odgovarajucu grupu
                }
            }
        }
        std::cout << "Nema dovoljno mesta za " << n << " sedista jedan do drugog." << std::endl;
    }

    // Funkcija koja obradjuje klikove miša
    void processMouseInput(GLFWwindow* window, int screenWidth, int screenHeight) {
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        if (state == GLFW_PRESS && oldLeftClickState == GLFW_RELEASE) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // NDC Konverzija
            float ndcX = (2.0f * (float)mouseX / (float)screenWidth) - 1.0f;
            float ndcY = 1.0f - (2.0f * (float)mouseY / (float)screenHeight);

            for (Seat& s : seats) {
                // AABB Collision
                if (ndcX >= s.x && ndcX <= (s.x + s.width) && ndcY >= s.y && ndcY <= (s.y + s.height)) {
                    // Logika klika:
                    // Ako je FREE -> RESERVED
                    // Ako je RESERVED -> FREE
                    // Ako je SOLD -> Ne radimo nista (vec je prodato)
                    if (s.state == FREE) {
                        s.state = RESERVED;
                    }
                    else if (s.state == RESERVED) {
                        s.state = FREE;
                    }
                    break;
                }
            }
        }
        oldLeftClickState = (state == GLFW_PRESS);
    }

    // Nova funkcija za tastaturu (Task 1-9)
    void processKeyboardInput(GLFWwindow* window) {
        // Proveravamo tastere od '1' do '9'
        for (int i = 1; i <= 9; i++) {
            // GLFW mapira tastere GLFW_KEY_0 do GLFW_KEY_9 redom
            int key = GLFW_KEY_0 + i;

            int state = glfwGetKey(window, key);

            // Detekcija klika (samo jednom kad se pritisne)
            if (state == GLFW_PRESS && oldKeyStates[i] == false) {
                buyTickets(i); // Pokusaj kupovinu i karata
            }

            oldKeyStates[i] = (state == GLFW_PRESS);
        }
    }
};

#endif