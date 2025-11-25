#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D skydomeTexture; // Nuestra textura HDR

void main()
{
    // Muestra la textura HDR tal cual (se "quemará" en 1.0, pero es lo esperado sin tone mapping)
    FragColor = vec4(texture(skydomeTexture, TexCoords).rgb, 1.0);
}