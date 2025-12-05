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

    // 2. Kreiranje prozora
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Kreiramo prozor (stavio sam fiksnu velicinu radi testiranja, a ti vrati mode->width/height ako zelis full screen)
    // Bolje je za testiranje koristiti manji prozor da ne blokira ceo ekran ako pukne.
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Bioskopska Sala", primaryMonitor, NULL);
    // Ako zelis windowed mode za testiranje, koristi ovo ispod umesto linije iznad:
    // GLFWwindow* window = glfwCreateWindow(1280, 720, "Bioskopska Sala", NULL, NULL);

    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");

    glfwMakeContextCurrent(window);

    // 3. Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    // 4. Podesavanje OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dohvatamo stvarnu velicinu prozora (bitno za HighDPI ekrane)
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // -----------------------------------------------------------------------
    // PRIPREMA PODATAKA
    // -----------------------------------------------------------------------

    // BITNO: Provera da li su fajlovi tu
    unsigned int shaderProgram = createShader("basic.vert", "basic.frag");
    // createShader vraca neki ID, ali ako je Util.cpp dobro napisan, ispisace gresku u konzoli.
    // Ovde cemo dodati, za svaki slucaj, proveru (iako createShader uvek vrati nesto).
    glUseProgram(shaderProgram); // Aktiviramo odmah da vidimo da li puca

    // Jedinicni kvadrat (0,0 do 1,1)
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
    // Provera teksture
    if (texturePotpis == 0) {
        std::cout << "UPOZORENJE: 'potpis.png' nije pronadjen ili nije ucitan!" << std::endl;
    }

    // Uniforme
    int uPosLoc = glGetUniformLocation(shaderProgram, "uPos");
    int uSizeLoc = glGetUniformLocation(shaderProgram, "uSize");
    int uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
    int uUseTextureLoc = glGetUniformLocation(shaderProgram, "uUseTexture");
    int uTexLoc = glGetUniformLocation(shaderProgram, "uTex");

    glUniform1i(uTexLoc, 0); // Tekstura unit 0

    // --- INICIJALIZACIJA MENADZERA SEDISTA ---
    SeatManager seatManager;
    // -----------------------------------------

    double lastTime = glfwGetTime();
    double timer = lastTime;
    double deltaTime = 0;
    double nowTime = 0;

    // -----------------------------------------------------------------------
    // GLAVNA PETLJA
    // -----------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {

        // 1. OBRADA DOGADJAJA (PRVO OVO!)
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 2. FPS LIMITER (Bolja logika)
        nowTime = glfwGetTime();
        deltaTime = nowTime - lastTime;

        // Ako nije proslo dovoljno vremena za sledeci frejm (1/75 sekunde), preskoci crtanje
        // Ovo sprecava da CPU gori, ali omogucava windowsu da dise
        if (deltaTime < TARGET_FRAME_TIME) {
            continue;
        }
        lastTime = nowTime;

        // 3. LOGIKA APLIKACIJE
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        // Ako se prozor minimizuje, w i h budu 0, sto moze izazvati bug sa deljenjem nulom u NDC konverziji
        if (w > 0 && h > 0) {
            seatManager.processInput(window, w, h);
        }

        // 4. CRTANJE
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // A) Platno
        glUniform1i(uUseTextureLoc, 0);
        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glUniform2f(uPosLoc, -0.6f, 0.6f);
        glUniform2f(uSizeLoc, 1.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // B) Ulaz
        glUniform4f(uColorLoc, 0.2f, 0.2f, 0.8f, 1.0f);
        glUniform2f(uPosLoc, -0.95f, 0.6f);
        glUniform2f(uSizeLoc, 0.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // C) SEDISTA
        for (const Seat& s : seatManager.seats) {
            if (s.isReserved) {
                glUniform4f(uColorLoc, 1.0f, 1.0f, 0.0f, 1.0f); // Zuto
            }
            else {
                glUniform4f(uColorLoc, 0.0f, 0.5f, 1.0f, 1.0f); // Plavo
            }
            glUniform2f(uPosLoc, s.x, s.y);
            glUniform2f(uSizeLoc, s.width, s.height);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // D) Overlay (Tamni ekran pozadi)
        glUniform4f(uColorLoc, 0.1f, 0.1f, 0.1f, 0.5f);
        glUniform2f(uPosLoc, -1.0f, -1.0f);
        glUniform2f(uSizeLoc, 2.0f, 2.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // E) Potpis
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

    // Ciscenje
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}