#version 150

in vec2 vVel;
in float vRand;
out vec4 fragColor;

void main(){
    float speed = length(vVel);
    float shade = clamp(0.35 + speed * 5.0, 0.0, 1.0);
    // circular falloff for point sprite
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float alpha = 1.0 - smoothstep(0.65, 1.0, length(p));
    alpha *= 0.35 + vRand * 0.4; // boost alpha for visibility
    fragColor = vec4(vec3(shade), alpha);
}
