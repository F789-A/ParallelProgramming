#version 420 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D ScreenTexture;

void main()
{
    FragColor = texture(ScreenTexture, TexCoords);
}