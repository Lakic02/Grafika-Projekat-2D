#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>

// Tvoji hederi
#include "../Header/Util.h"
#include "../Header/SeatManager.h"

// Konstante
const double TARGET_FPS = 75.0;
const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

int main() {
    // 1. Inicijalizacija GLFW
    if (!glfwInit()) return endProgram("GLFW nije uspeo da se inicijalizuje.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Kreiranje prozora - FULL SCREEN (Zahtev zadatka)
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Za pravi full screen koristi:
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Bioskopska Sala", primaryMonitor, NULL);
    //samo za test:
	//GLFWwindow* window = glfwCreateWindow(1280, 720, "Bioskopska Sala", NULL, NULL);    

    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");

    glfwMakeContextCurrent(window);

    // 3. Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

	// 4. Podesavanje kursora
    GLFWcursor* cursor = loadImageToCursor("cursor.png");
    if (!cursor) {
        std::cout << "Cursor nije ucitan! Proveri putanju i format." << std::endl;
    }
    else {
        std::cout << "Cursor ucitan uspesno." << std::endl;
        glfwSetCursor(window, cursor);
    }

    // 5. Podesavanje OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // -----------------------------------------------------------------------
    unsigned int shaderProgram = createShader("basic.vert", "basic.frag");
    glUseProgram(shaderProgram);

    // Jedinicni kvadrat
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
    if (texturePotpis == 0) std::cout << "UPOZORENJE: 'potpis.png' fali!" << std::endl;

    // Uniforme
    int uPosLoc = glGetUniformLocation(shaderProgram, "uPos");
    int uSizeLoc = glGetUniformLocation(shaderProgram, "uSize");
    int uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
    int uUseTextureLoc = glGetUniformLocation(shaderProgram, "uUseTexture");
    int uTexLoc = glGetUniformLocation(shaderProgram, "uTex");
    glUniform1i(uTexLoc, 0);

    SeatManager seatManager;

    double lastTime = glfwGetTime();
    double nowTime = 0;
    double deltaTime = 0;

    // -----------------------------------------------------------------------
    // GLAVNA PETLJA
    // -----------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents(); // Obrada dogadjaja

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Frame Limiter (75 FPS)
        nowTime = glfwGetTime();
        deltaTime = nowTime - lastTime;
        if (deltaTime < TARGET_FRAME_TIME) continue;
        lastTime = nowTime;

        // --- INPUT PROCESIRANJE ---
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        if (w > 0 && h > 0) {
            // Mis (za rezervaciju pojedinacnih)
            seatManager.processMouseInput(window, w, h);
            // Tastatura (za grupnu kupovinu 1-9)
            seatManager.processKeyboardInput(window);
        }

        // --- CRTANJE ---
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // 1. Platno
        glUniform1i(uUseTextureLoc, 0);
        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glUniform2f(uPosLoc, -0.6f, 0.6f);
        glUniform2f(uSizeLoc, 1.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 2. Ulaz
        glUniform4f(uColorLoc, 0.2f, 0.2f, 0.8f, 1.0f);
        glUniform2f(uPosLoc, -0.95f, 0.6f);
        glUniform2f(uSizeLoc, 0.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 3. SEDISTA (AZURIRANA LOGIKA BOJA)
        for (const Seat& s : seatManager.seats) {

            if (s.state == RESERVED) {
                // Rezervisano = ZUTO
                glUniform4f(uColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
            }
            else if (s.state == SOLD) {
                // Kupljeno = CRVENO
                glUniform4f(uColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
            }
            else {
                // Slobodno = PLAVO
                glUniform4f(uColorLoc, 0.0f, 0.5f, 1.0f, 1.0f);
            }

            glUniform2f(uPosLoc, s.x, s.y);
            glUniform2f(uSizeLoc, s.width, s.height);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // 4. Overlay (Tamni ekran)
        glUniform4f(uColorLoc, 0.1f, 0.1f, 0.1f, 0.5f);
        glUniform2f(uPosLoc, -1.0f, -1.0f);
        glUniform2f(uSizeLoc, 2.0f, 2.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 5. Potpis (Ime i prezime)
        if (texturePotpis != 0) {
            glUniform1i(uUseTextureLoc, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texturePotpis);

            glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 0.8f);
            glUniform2f(uPosLoc, 0.5f, -0.9f);
            glUniform2f(uSizeLoc, 0.4f, 0.2f);
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