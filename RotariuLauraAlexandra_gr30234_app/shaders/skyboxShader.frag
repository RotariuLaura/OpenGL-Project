#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform int nightCond;

void main()
{
    if (nightCond)
    {
        //color = vec4(0.0, 0.0, 0.2, 1.0);
        vec4 nightColor = vec4(0.0, 0.0, 0.2, 1.0);
        vec4 textureColor = texture(skybox, textureCoordinates);
        color = textureColor * nightColor;
    }
    else
        color = texture(skybox, textureCoordinates);
}
