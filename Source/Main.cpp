#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

#include "../Header/Util.h"
#include "../Header/SeatManager.h"
#include "../Header/PersonManager.h"
#include "../Header/CinemaSimulator.h" 

const double TARGET_FPS = 75.0;
const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

int main() {
    srand(static_cast<unsigned int>(time(0)));

    if (!glfwInit()) return endProgram("GLFW greska.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Bioskop Simulator", primaryMonitor, NULL);

    if (window == NULL) return endProgram("Prozor greska.");

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) return endProgram("GLEW greska.");

    GLFWcursor* cursor = loadImageToCursor("cursor.png");
    if (cursor) glfwSetCursor(window, cursor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // --- PROMENA BOJE POZADINE ---
    // #FFA878 -> R:1.0, G:0.659, B:0.471
    glClearColor(1.0f, 0.659f, 0.471f, 1.0f);

    unsigned int shaderProgram = createShader("basic.vert", "basic.frag");
    glUseProgram(shaderProgram);

    // Kvadrat geometrija
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

    SeatManager seatManager;
    PersonManager personManager;
    CinemaSimulator simulator;

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {

        double nowTime = glfwGetTime();
        double deltaTime = nowTime - lastTime;
        if (deltaTime < TARGET_FRAME_TIME) continue;
        lastTime = nowTime;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Input samo u IDLE
        if (simulator.currentState == IDLE) {
            int w, h;
            glfwGetWindowSize(window, &w, &h);
            seatManager.processMouseInput(window, w, h);
            seatManager.processKeyboardInput(window);

            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                simulator.startProjection(personManager, seatManager);
            }
        }

        simulator.update(deltaTime, personManager, seatManager);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // Platno
        simulator.drawScreen(uPosLoc, uSizeLoc, uColorLoc, uUseTextureLoc);

        // Vrata
        glUniform1i(uUseTextureLoc, 0);
        simulator.drawDoors(uPosLoc, uSizeLoc, uColorLoc);

        // Sedista
        for (const Seat& s : seatManager.seats) {
            if (s.state == RESERVED) glUniform4f(uColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
            else if (s.state == SOLD) glUniform4f(uColorLoc, 0.8f, 0.0f, 0.0f, 1.0f);
            else glUniform4f(uColorLoc, 0.0f, 0.6f, 1.0f, 1.0f);

            glUniform2f(uPosLoc, s.x, s.y);
            glUniform2f(uSizeLoc, s.width, s.height);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Ljudi
        if (simulator.currentState != IDLE) {
            personManager.draw(shaderProgram, uPosLoc, uSizeLoc, uColorLoc, uUseTextureLoc);
        }

        // Overlay (Tamna zavesa) - Prikazi samo kada je IDLE
        if (simulator.shouldDrawOverlay()) {
            glUniform1i(uUseTextureLoc, 0);
            glUniform4f(uColorLoc, 0.0f, 0.0f, 0.0f, 0.6f);
            glUniform2f(uPosLoc, -1.0f, -1.0f);
            glUniform2f(uSizeLoc, 2.0f, 2.0f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Potpis
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