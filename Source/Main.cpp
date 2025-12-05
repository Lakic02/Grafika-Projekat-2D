#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime> // Za random seed

#include "../Header/Util.h"
#include "../Header/SeatManager.h"
#include "../Header/stb_image.h"
#include "../Header/PersonManager.h"// <--- NOVI INCLUDE

const double TARGET_FPS = 75.0;
const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

int main() {
    // Inicijalizacija Random seed-a
    srand(static_cast<unsigned int>(time(0)));

    if (!glfwInit()) return endProgram("GLFW nije uspeo da se inicijalizuje.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // FULL SCREEN
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Bioskopska Sala 8x9", primaryMonitor, NULL);

    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    // Kursor
    GLFWcursor* cursor = loadImageToCursor("cursor.png");
    if (cursor) glfwSetCursor(window, cursor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Pozadina tamno siva
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    unsigned int shaderProgram = createShader("basic.vert", "basic.frag");
    glUseProgram(shaderProgram);

    // Geometrija (Kvadrat)
    float vertices[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texturePotpis = loadImageToTexture("potpis.png");

    int uPosLoc = glGetUniformLocation(shaderProgram, "uPos");
    int uSizeLoc = glGetUniformLocation(shaderProgram, "uSize");
    int uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
    int uUseTextureLoc = glGetUniformLocation(shaderProgram, "uUseTexture");
    int uTexLoc = glGetUniformLocation(shaderProgram, "uTex");
    glUniform1i(uTexLoc, 0);

    // --- MENADZERI ---
    SeatManager seatManager;
    PersonManager personManager; // <--- Instanca menadzera ljudi

    // --- STANJE APLIKACIJE ---
    bool projectionStarted = false; // Da li je pritisnut ENTER?

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {

        double nowTime = glfwGetTime();
        double deltaTime = nowTime - lastTime;
        if (deltaTime < TARGET_FRAME_TIME) continue;
        lastTime = nowTime;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // --- DETEKCIJA ENTERA ---
        if (!projectionStarted && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            projectionStarted = true;
            // Generisemo ljude na osnovu stanja sedista
            personManager.spawnPeople(seatManager);
            std::cout << "Projekcija pocinje! Vrata otvorena." << std::endl;
        }

        // --- INPUT (Samo ako projekcija NIJE pocela) ---
        int w, h;
        glfwGetWindowSize(window, &w, &h);

        if (!projectionStarted) {
            // Dozvoljena kupovina i rezervacija
            seatManager.processMouseInput(window, w, h);
            seatManager.processKeyboardInput(window);
        }
        else {
            // Projekcija traje: update-ujemo kretanje ljudi
            personManager.update(deltaTime);
        }

        // --- CRTANJE ---
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // 1. Platno
        glUniform1i(uUseTextureLoc, 0);
        glUniform4f(uColorLoc, 0.9f, 0.9f, 0.9f, 1.0f);
        glUniform2f(uPosLoc, -0.6f, 0.6f);
        glUniform2f(uSizeLoc, 1.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 2. Vrata (Logika boje)
        if (projectionStarted) {
            // Otvorena vrata -> Svetlo Plava (Cijan)
            glUniform4f(uColorLoc, 0.0f, 1.0f, 1.0f, 1.0f);
        }
        else {
            // Zatvorena vrata -> Tamno Plava
            glUniform4f(uColorLoc, 0.1f, 0.1f, 0.6f, 1.0f);
        }
        // Vrata su gore levo
        glUniform2f(uPosLoc, -0.98f, 0.6f);
        glUniform2f(uSizeLoc, 0.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 3. Sedista
        for (const Seat& s : seatManager.seats) {
            if (s.state == RESERVED) glUniform4f(uColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
            else if (s.state == SOLD) glUniform4f(uColorLoc, 0.8f, 0.0f, 0.0f, 1.0f);
            else glUniform4f(uColorLoc, 0.0f, 0.6f, 1.0f, 1.0f);

            glUniform2f(uPosLoc, s.x, s.y);
            glUniform2f(uSizeLoc, s.width, s.height);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // 4. Ljudi (Crtamo ih samo ako je projekcija pocela)
        if (projectionStarted) {
            personManager.draw(shaderProgram, uPosLoc, uSizeLoc, uColorLoc, uUseTextureLoc);
        }

        // 5. Overlay (Tamni pravougaonik) - CRTA SE SAMO DOK PROJEKCIJA NIJE POCELA
        if (!projectionStarted) {
            glUniform1i(uUseTextureLoc, 0);
            glUniform4f(uColorLoc, 0.0f, 0.0f, 0.0f, 0.6f); // Crna, providna
            glUniform2f(uPosLoc, -1.0f, -1.0f);
            glUniform2f(uSizeLoc, 2.0f, 2.0f); // Preko celog ekrana
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // 6. Potpis studenta (Uvek prisutan)
        if (texturePotpis != 0) {
            glUniform1i(uUseTextureLoc, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texturePotpis);

            glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 0.8f);
            glUniform2f(uPosLoc, 0.5f, -0.9f);
            glUniform2f(uSizeLoc, 0.45f, 0.2f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}