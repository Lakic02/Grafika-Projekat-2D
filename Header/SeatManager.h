#pragma once
#ifndef SEAT_MANAGER_H
#define SEAT_MANAGER_H

#include <vector>
#include <iostream> 
#include <GL/glew.h>
#include <GLFW/glfw3.h>

enum SeatState {
    FREE,       // Slobodno (Plavo)
    RESERVED,   // Rezervisano (Zuto)
    SOLD        // Kupljeno (Crveno)
};

struct Seat {
    float x;
    float y;
    float width;
    float height;
    SeatState state;
};

class SeatManager {
public:
    std::vector<Seat> seats;
    bool oldLeftClickState;
    bool oldKeyStates[10];

    // Konstante za dimenzije
    const int ROWS = 8;
    const int COLS = 9; // <--- PROMENA: SADA JE 9 KOLONA

    SeatManager() {
        oldLeftClickState = false;
        for (int i = 0; i < 10; i++) oldKeyStates[i] = false;
        initSeats();
    }

    void initSeats() {
        // Prilagodjavanje pozicije da bi sirih 9 sedista bilo centrirano
        // Sirina jednog bloka je oko 1.14 (8 * 0.13 + 0.1). 
        // Centar je 0, pa krecemo od -0.57.
        float startX = -0.57f;
        float startY = 0.3f;
        float gapX = 0.13f;
        float gapY = 0.15f;
        float seatW = 0.1f;
        float seatH = 0.1f;

        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++) // <--- PROMENA: Ide do 9
            {
                Seat s;
                s.x = startX + j * gapX;
                s.y = startY - i * gapY;
                s.width = seatW;
                s.height = seatH;
                s.state = FREE;
                seats.push_back(s);
            }
        }
    }

    void buyTickets(int n) {
        if (n <= 0 || n > COLS) return; // Zastita: ne mozemo kupiti vise od 9

        // Trazimo od poslednjeg reda ka prvom
        for (int row = ROWS - 1; row >= 0; --row) {

            // Unutar reda trazimo N susednih.
            // Krecemo od krajnje desne kolone (indeks COLS - 1, tj. 8)
            for (int col = COLS - 1; col >= n - 1; --col) {
                bool foundGroup = true;

                // Proveravamo grupu od N sedista na levo
                for (int k = 0; k < n; ++k) {
                    int checkCol = col - k;
                    // FORMULA ZA INDEKS U 1D NIZU: row * BrojKolona + col
                    int index = row * COLS + checkCol;

                    if (seats[index].state != FREE) {
                        foundGroup = false;
                        break;
                    }
                }

                if (foundGroup) {
                    std::cout << "Kupovina uspesna! Red: " << row + 1 << ", " << n << " sedista." << std::endl;
                    for (int k = 0; k < n; ++k) {
                        int index = row * COLS + (col - k);
                        seats[index].state = SOLD;
                    }
                    return;
                }
            }
        }
        std::cout << "Nema dovoljno mesta za " << n << " sedista jedan do drugog." << std::endl;
    }

    void processMouseInput(GLFWwindow* window, int screenWidth, int screenHeight) {
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        if (state == GLFW_PRESS && oldLeftClickState == GLFW_RELEASE) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            float ndcX = (2.0f * (float)mouseX / (float)screenWidth) - 1.0f;
            float ndcY = 1.0f - (2.0f * (float)mouseY / (float)screenHeight);

            for (Seat& s : seats) {
                if (ndcX >= s.x && ndcX <= (s.x + s.width) && ndcY >= s.y && ndcY <= (s.y + s.height)) {
                    if (s.state == FREE) s.state = RESERVED;
                    else if (s.state == RESERVED) s.state = FREE;
                    break;
                }
            }
        }
        oldLeftClickState = (state == GLFW_PRESS);
    }

    void processKeyboardInput(GLFWwindow* window) {
        // Tasteri 1-9
        for (int i = 1; i <= 9; i++) {
            int key = GLFW_KEY_0 + i;
            int state = glfwGetKey(window, key);

            if (state == GLFW_PRESS && oldKeyStates[i] == false) {
                buyTickets(i);
            }
            oldKeyStates[i] = (state == GLFW_PRESS);
        }
    }
};

#endif