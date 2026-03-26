uniform sampler2D texture;
uniform vec2 u_resolution;
uniform float u_time;
uniform float u_alpha;
uniform float u_mix;

#define NUM_EXPLOSIONS 5.0
#define NUM_PARTICLES 75.0

vec2 Hash12(float t) {
    float x = fract(sin(t * 674.3) * 453.2);
    float y = fract(sin((t + x) * 714.3) * 263.2);

    return vec2(x, y);
}

vec2 Hash12_Polar(float t) {
    float p_Angle = fract(sin(t * 674.3) * 453.2) * 6.2832;
    float p_Dist = fract(sin((t + p_Angle) * 714.3) * 263.2);

    return vec2(sin(p_Angle), cos(p_Angle)) * p_Dist;
}

float Explosion(vec2 uv, float t) {
    float sparks = 0.0;
 
    for(float i = 0.0; i < NUM_PARTICLES; i++) {    
        vec2 dir = Hash12_Polar(i + 1.0) * 0.5;
        float dist = length(uv - dir * t);
        float brightness = mix(0.0005, 0.0005, smoothstep(0.05, 0.0, t));
        
        brightness *= sin(t * 20.0 + i) * 0.5 + 0.5; 
        brightness*= smoothstep(1.0, 0.6, t);
        sparks += brightness / dist;
    }

    return sparks;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution.xy) / u_resolution.y;

    vec3 col = vec3(0);
    
    for(float i = 0.0; i < NUM_EXPLOSIONS; i++) {
        float t = u_time + i / NUM_EXPLOSIONS;
        float ft = floor(t);
        vec3 color = sin(4.0 * vec3(0.34, 0.54, 0.43) * ft) * 0.25 + 0.75;
       
        vec2 offset = Hash12(i + 1.0 + ft) - 0.5;
        offset *= vec2(1.77, 1.0);
        
        col += Explosion(uv - offset, fract(t)) * color;
    }
   
    col *= 2.0;

    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    col = mix(col, pixel.rgb, u_mix);

    gl_FragColor = vec4(col, u_alpha);
}