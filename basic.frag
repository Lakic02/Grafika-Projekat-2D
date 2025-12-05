#version 330 core

in vec2 chTex;
out vec4 outCol;

uniform vec4 uColor;        // Boja objekta (R, G, B, A)
uniform sampler2D uTex;     // Tekstura
uniform bool uUseTexture;   // Da li koristimo teksturu ili boju?

void main()
{
    if (uUseTexture)
    {
        // Ako koristimo teksturu, uzimamo boju sa slike
        vec4 texColor = texture(uTex, chTex);
        // Mnozimo sa uColor ako zelimo da "toniramo" sliku, ili ako je uColor bela, slika je originalna
        // Takodje, ovo omogucava providnost ako je tekstura transparentna
        outCol = texColor; 
    }
    else
    {
        // Inace cista boja
        outCol = uColor;
    }
}