#version 330 core

layout(location = 0) in vec2 inPos; // Ulazne koordinate (samo X i Y su nam dovoljne za 2D)
layout(location = 1) in vec2 inTex; // Teksturne koordinate

out vec2 chTex; // Saljemo teksturne koordinate u fragment shader

uniform vec2 uPos;   // Gde se objekat nalazi (X, Y) - NDC koordinate (-1 do 1)
uniform vec2 uSize;  // Koliki je objekat (Sirina, Visina)

void main()
{
    // Formula: (Originalna_Pozicija * Velicina) + Pozicija
    // Ovo simulira model matricu za jednostavne 2D pravougaonike
    gl_Position = vec4(inPos * uSize + uPos, 0.0, 1.0);
    
    chTex = inTex;
}