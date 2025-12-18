#version 150

uniform sampler2D posTex;   // RG = pos, BA = vel
uniform sampler2D maskTex;  // NDI mask
uniform vec2 posRes;
uniform vec2 screenRes;
uniform float dt;
uniform float gravity;
uniform float threshold;
uniform float noiseStrength;
uniform int collide;
uniform int invertMask;
uniform float time;

in vec2 vTexCoord;
out vec4 fragColor;

float hash(vec2 p){
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main(){
    vec4 data = texture(posTex, vTexCoord);
    vec2 pos = data.rg;
    vec2 vel = data.ba;

    float maskInfluence = 0.0;
    if(collide == 1){
        vec2 maskUV = clamp(pos, vec2(0.0), vec2(1.0));
        vec3 maskSample = texture(maskTex, maskUV).rgb;
        float lum = dot(maskSample, vec3(0.299, 0.587, 0.114));
        if(invertMask == 1) lum = 1.0 - lum;
        maskInfluence = smoothstep(threshold, threshold + 0.1, lum);
    }

    // gravity and drag
    vel += vec2(0.0, gravity) * dt;
    vel *= 0.99;

    // turbulence
    vec2 t = vec2(hash(vTexCoord * 3.17 + time),
                  hash(vTexCoord * 5.31 - time)) - 0.5;
    vel += t * noiseStrength * 0.22;

    // jitter
    float n = hash(vTexCoord + time * 1.3);
    vel.x += (n - 0.5) * noiseStrength * dt;
    vel.y += (hash(vTexCoord * 7.7 + time * 0.5) - 0.5) * noiseStrength * 0.04;

    // bounce / splash on bright
    if(collide == 1 && maskInfluence > 0.5 && vel.y > 0.0){
        float bounce = mix(0.35, 0.65, maskInfluence);
        vel.y *= -bounce;
        float nn = hash(vTexCoord + time * 0.7);
        vel.x += (nn - 0.5) * (0.35 + maskInfluence * 0.5);
        vel.y += gravity * 0.5 * dt;
        pos.y = clamp(pos.y - 0.005, 0.0, 1.0);
    }

    // clamp velocity
    vel.y = clamp(vel.y, -3.0, 3.5);
    vel.x = clamp(vel.x, -2.5, 2.5);

    pos += vel * dt;

    // small offset to break rows
    pos.y += (hash(vTexCoord * 9.1 + time * 0.9) - 0.5) * 0.0025;

    // respawn at top if out of bounds
    if(pos.y > 1.02 || pos.y < -0.1){
        float r = hash(vec2(vTexCoord.y * 1.37, time * 0.5));
        pos = vec2(r, -0.05);
        vel = vec2(0.0, 0.0);
    }

    // wrap x softly
    if(pos.x < -0.05) pos.x = 1.0 + (pos.x + 0.05);
    if(pos.x > 1.05) pos.x = (pos.x - 1.05);

    fragColor = vec4(pos, vel);
}
