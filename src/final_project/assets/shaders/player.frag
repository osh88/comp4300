uniform float alpha;
uniform sampler2D texture;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    pixel = vec4(vec3((pixel.r + pixel.g + pixel.b) / 3.0), pixel.a); // Переводим в черно-белое
    if (alpha < 1.0 && pixel.a == 1.0) {
        pixel.a = alpha;
    }

    // multiply it by the color
    //gl_FragColor = gl_Color * pixel;
    gl_FragColor = pixel;
}