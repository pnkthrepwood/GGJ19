uniform sampler2D texture;

uniform float dayTime;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    float dayFactor = 1.0-cos(1.0 - abs(pow(sin(dayTime),10.0))) + 0.5;

    // multiply it by the color
    gl_FragColor = gl_Color * pixel * dayFactor;
}
