#version 150

in vec2 vVel;
in float vRand;
out vec4 fragColor;

uniform int renderSquares;

void main(){
    float speed = length(vVel);
    float shade = clamp(0.35 + speed * 5.0, 0.0, 1.0);
    vec2 p = gl_PointCoord;
    float alpha;
    if(renderSquares == 1){
        alpha = 0.35 + vRand * 0.4;
    } else {
        vec2 circ = p * 2.0 - 1.0;
        alpha = 1.0 - smoothstep(0.65, 1.0, length(circ));
        alpha *= 0.35 + vRand * 0.4;
    }
    fragColor = vec4(vec3(shade), alpha);
}
