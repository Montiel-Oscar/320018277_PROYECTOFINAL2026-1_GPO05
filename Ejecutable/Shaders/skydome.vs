#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords; // Tu clase Model envía esto

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view; // Esta será la matriz de vista SIN traslación

void main()
{
    TexCoords = aTexCoords;
    // Dibuja la esfera normal
    vec4 pos = projection * view * vec4(aPos, 1.0);
    // Este truco asegura que siempre esté en el fondo (z = 1.0)
    gl_Position = pos.xyww;
}