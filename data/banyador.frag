uniform sampler2D texture;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    if (pixel == vec4(1.0, 0.0, 1.0, 1.0)) {
      gl_FragColor = gl_Color;
    }
    else {
      gl_FragColor = pixel;
    }
}
