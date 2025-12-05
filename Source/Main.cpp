#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <direct.h>
#include <vector>

// Ukljucujemo tvoje hedere
#include "../Header/Util.h"

// Konstante
const double TARGET_FPS = 75.0;

int main()
{
    // 1. Inicijalizacija GLFW
    if (!glfwInit()) return endProgram("GLFW nije uspeo da se inicijalizuje.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Kreiranje Full Screen prozora (Zahtev projekta)
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Kreiramo prozor dimenzija monitora
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Bioskopska Sala", primaryMonitor, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");

    glfwMakeContextCurrent(window);

    // 3. Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    // 4. Podesavanje OpenGL Stanja
    glEnable(GL_BLEND); // Za providnost
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Podesavanje Viewport-a da pokrije ceo ekran
    glViewport(0, 0, mode->width, mode->height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Tamna pozadina bioskopa

    // -----------------------------------------------------------------------
    // PRIPREMA PODATAKA (BUFFERS & SHADERS)
    // -----------------------------------------------------------------------

    // Kompajliranje sejdera (koristeci tvoju Util funkciju)
    // Pretpostavljam da su fajlovi basic.vert i basic.frag u folderu "Shaders" ili istom folderu
    // Prilagodi putanju ako treba (npr. "Shaders/basic.vert")
    unsigned int shaderProgram = createShader("basic.vert", "basic.frag");

    // Definisanje jedinicnog kvadrata (Unit Quad)
    // Koordinate idu od 0 do 1 radi lakseg racunanja skaliranja
    // Format: X, Y (Pozicija), U, V (Tekstura)
    float vertices[] = {
        0.0f, 1.0f, 0.0f, 1.0f, // Gore levo
        0.0f, 0.0f, 0.0f, 0.0f, // Dole levo
        1.0f, 0.0f, 1.0f, 0.0f, // Dole desno

        0.0f, 1.0f, 0.0f, 1.0f, // Gore levo
        1.0f, 0.0f, 1.0f, 0.0f, // Dole desno
        1.0f, 1.0f, 1.0f, 1.0f  // Gore desno
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atribut 0: Pozicija (2 float-a)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atribut 1: Tekstura (2 float-a)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Ucitavanje tekstura
    // NAPOMENA: Moras napraviti/skinuti sliku "potpis.png" i staviti je u folder
    unsigned int texturePotpis = loadImageToTexture("potpis.png");

    if (texturePotpis == 0)
    {
        std::cout << "[GRESKA] Tekstura potpis NIJE ucitana! (texturePotpis = 0)" << std::endl;
        std::cout << "Proveri da li fajl 'potpis.png' postoji u WORKING DIRECTORY-u." << std::endl;
        char buff[FILENAME_MAX];
        _getcwd(buff, FILENAME_MAX);
        std::cout << "Trenutni working directory: " << buff << std::endl;
    }
    else
    {
        std::cout << "[INFO] Tekstura potpis je uspesno ucitana! (ID = " << texturePotpis << ")" << std::endl;
    }

    // Lokacije uniformi
    int uPosLoc = glGetUniformLocation(shaderProgram, "uPos");
    int uSizeLoc = glGetUniformLocation(shaderProgram, "uSize");
    int uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
    int uUseTextureLoc = glGetUniformLocation(shaderProgram, "uUseTexture");

    int uTexLoc = glGetUniformLocation(shaderProgram, "uTex");
    glUniform1i(uTexLoc, 0); // Kazemo sejderu da sempler "uTex" cita sa Texture Unit 0

    // Frame limiter promenljive
    double lastTime = glfwGetTime();
    double timer = lastTime;
    double deltaTime = 0;
    double nowTime = 0;

    // -----------------------------------------------------------------------
    // GLAVNA PETLJA
    // -----------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // 1. Frame Limiter Logika (75 FPS)
        nowTime = glfwGetTime();
        while (nowTime < lastTime + 1.0 / TARGET_FPS) {
            nowTime = glfwGetTime(); // Cekamo...
            // (Ovde bi mogao ici sleep da ne trosi CPU, ali busy-wait je precizniji)
        }
        lastTime = nowTime;

        // Izlaz na ESC
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 2. Ciscenje ekrana
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // -------------------------------------------------
        // CRTANJE BIOSKOPSKE SALE
        // -------------------------------------------------

        // --- 1. Bioskopsko platno (Beli pravougaonik gore) ---
        // Postavljamo da ne koristi teksturu
        glUniform1i(uUseTextureLoc, 0);
        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Bela boja

        // Pozicija i velicina su u NDC (-1 do 1).
        // 0,0 je centar. -1,-1 dole levo. 1,1 gore desno.
        // Zelimo pravougaonik gore u sredini.
        glUniform2f(uPosLoc, -0.6f, 0.6f); // X: -0.6, Y: 0.6
        glUniform2f(uSizeLoc, 1.2f, 0.3f); // Sirina: 1.2, Visina: 0.3
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- 2. Ulaz u salu (Gore levo) ---
        glUniform4f(uColorLoc, 0.2f, 0.2f, 0.8f, 1.0f); // Plava boja (Vrata)
        glUniform2f(uPosLoc, -0.95f, 0.6f);
        glUniform2f(uSizeLoc, 0.2f, 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- 3. Sedista (Crvena, 50+ komada) ---
        glUniform4f(uColorLoc, 0.8f, 0.1f, 0.1f, 1.0f); // Crvena sedista

        // Pravimo mrezu sedista (npr. 8 redova po 8 sedista = 64 sedista)
        float startX = -0.5f;
        float startY = 0.3f;
        float gapX = 0.13f;
        float gapY = 0.15f;
        float seatW = 0.1f;
        float seatH = 0.1f;

        for (int i = 0; i < 8; i++) // Redovi
        {
            for (int j = 0; j < 8; j++) // Kolone
            {
                float x = startX + j * gapX;
                float y = startY - i * gapY;

                glUniform2f(uPosLoc, x, y);
                glUniform2f(uSizeLoc, seatW, seatH);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        // --- 4. Tamnosivi pravougaonik preko celog ekrana (Overlay) ---
        // Zahtev: "tamnosivi pravougaonik sa providnošću 0,5"
        glUniform4f(uColorLoc, 0.1f, 0.1f, 0.1f, 0.5f); // RGB (0.1,0.1,0.1), Alpha 0.5
        glUniform2f(uPosLoc, -1.0f, -1.0f); // Od donjeg levog ugla
        glUniform2f(uSizeLoc, 2.0f, 2.0f);  // Preko celog ekrana (velicina 2x2 u NDC)
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- 5. Potpis studenta (Tekstura) ---
        // Zahtev: "U proizvoljnom uglu ekrana poluprovidna tekstura"
        if (texturePotpis != 0) // Samo ako je uspesno ucitana
        {
            glUniform1i(uUseTextureLoc, 1); // Ukljucujemo teksturiranje
            glBindTexture(GL_TEXTURE_2D, texturePotpis);

            // Postavljamo providnost teksture na 0.8 (malo providno)
            glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 0.8f);

            // Crtamo u donjem desnom uglu
            glUniform2f(uPosLoc, 0.5f, -0.9f);
            glUniform2f(uSizeLoc, 0.4f, 0.2f); // Prilagodi odnos stranica prema slici
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Zamena bafera i poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Ciscenje
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}