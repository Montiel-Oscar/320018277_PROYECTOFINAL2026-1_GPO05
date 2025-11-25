#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform bool cloudsMoving;
uniform float speed;
uniform float tileFactor;

void main()
{
    vec2 uv = aTexCoords * tileFactor;

    if (cloudsMoving) {
        float offset = mod(time * speed, 1.0);  // Loop perfecto: 0.0 a 1.0
        TexCoords = vec2(uv.x - offset, uv.y);  // Mueve hacia la izquierda
    } else {
        TexCoords = uv;
    }

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}