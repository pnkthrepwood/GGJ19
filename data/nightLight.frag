uniform sampler2D texture;

uniform float dayTime;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    float dayFactor = (sin(dayTime) + 1.0) / 2.0;

    // multiply it by the color
    gl_FragColor = gl_Color * pixel * dayFactor;
}
