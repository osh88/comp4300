// Simple Fireworks - by moranzcw - 2021
// Email: moranzcw@gmail.com
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

uniform sampler2D texture;
uniform vec2 u_resolution;
uniform float u_time;
uniform float u_alpha;
uniform float u_mix;

#define Gravitational_ACCE vec2(0, -0.2)
#define NumSpark 20.0
#define NumFirework 3.0
#define Period 3.0

float rand(float co)
{ 
    return fract(sin(co*(91.3458)) * 47453.5453);
}

vec3 spark(vec2 coord, vec2 origin, vec3 sparkColor, vec2 velocity, float periodTime, float seed)
{
    vec2 displacement = velocity * periodTime + 0.5 * Gravitational_ACCE * periodTime * periodTime;
    vec2 sparkPos = origin + displacement;
    float len = length(coord - sparkPos);
    
    // glow color
    vec3 color = sparkColor * pow(smoothstep(0.05, 0.001, len),8.0);
    // core color
    color += vec3(1.0) * smoothstep(0.005, 0.001, len);
    // attenuation
    color *= smoothstep(Period, 0.0, periodTime);
    // twinkle
    color *= sin(periodTime*50.0 + seed)*0.4 + 0.6;
    return color;
}

vec3 firework(vec2 coord, float currentPeriod, float periodTime)
{   
    vec2 origin;
    origin.x = (0.2 + 0.6*rand(currentPeriod)) * u_resolution.x/u_resolution.y ;
    origin.y = (0.4 + 0.4*rand(currentPeriod));

    vec3 color = vec3(0.0);
    for(float i=0.0; i < NumSpark; i++) 
    {
        vec2 velocity;
        velocity.x = 0.5*(rand(currentPeriod+i*0.4)-0.5);
        velocity.y = 0.5*(rand(currentPeriod+i*0.5)-0.5);
        velocity = normalize(velocity);
        velocity *= 0.2 + rand(currentPeriod+i*0.6) * 0.3;
        
        vec3 sparkColor;
        sparkColor.r = 0.3+0.7*rand(currentPeriod+i*0.1);
        sparkColor.g = 0.3+0.7*rand(currentPeriod+i*0.2);
        sparkColor.b = 0.3+0.7*rand(currentPeriod+i*0.3);
        color += spark(coord, origin, sparkColor, velocity, periodTime, i);
    }
    return color;
}

vec3 background(float yCoord) 
{	    
    return mix(vec3(0.1515, 0.2375, 0.5757), vec3(0.0546, 0.0898, 0.1953), yCoord);
}

void main()
{
    vec2 coord = gl_FragCoord.xy/u_resolution.y;

    vec3 color = vec3(0.0);
    for(float i=0.0; i < NumFirework; i++)
    {
        float timeOffset = Period / NumFirework * (0.5 + rand(i));
        float time = u_time + i*timeOffset;
        float periodTime = mod(time,Period);
        
        float periodOffset = i*10.0; // for random
        float currentPeriod = floor(time/Period + periodOffset);
        
        color += firework(coord, currentPeriod, periodTime);
    }
    color = background(coord.y)+color;

    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    color = mix(color, pixel.rgb, u_mix);

    gl_FragColor = vec4(color, u_alpha);
}