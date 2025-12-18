#version 150

uniform sampler2D posTex;
uniform float pointSize;
uniform float time;
uniform int renderSquares;

in vec4 position;
in vec2 texcoord;

out vec2 vVel;
out float vRand;

float hash(vec2 p){
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main(){
    vec2 uv = texcoord;
    vec4 data = texture(posTex, uv);
    vec2 pos = data.xy;
    vVel = data.zw;

    // small jitter to break banding
    float j = (hash(uv + time) - 0.5) * 0.0045;
    pos.y += j;
    vRand = hash(uv * 13.37 + time);

    // convert 0..1 to clip space, flip y correctly
    vec2 clip = vec2(pos.x * 2.0 - 1.0, (1.0 - pos.y) * 2.0 - 1.0);
    gl_Position = vec4(clip, 0.0, 1.0);
    gl_PointSize = pointSize;
}
