#version 150

in vec4 position;
in vec2 texcoord;

out vec2 vTexCoord;

void main(){
    vTexCoord = texcoord;
    // incoming position is in pixel space; rebuild clip coords from texcoord
    vec2 clip = texcoord * 2.0 - 1.0;
    gl_Position = vec4(clip, 0.0, 1.0);
}
