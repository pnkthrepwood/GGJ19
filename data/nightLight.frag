uniform sampler2D texture;

uniform vec3 day_color;
uniform vec3 sun_set_color;
uniform vec3 night_color;

uniform float dayTime;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    float dayFactor = 1.0-cos(1.0 - abs(pow(sin(dayTime),10.0))) + 0.5;

    float factor_day_to_sunset = (dayFactor - 0.75f) / (1.f - 0.75f);
    float factor_sunset_to_night = (dayFactor - 0.5f) / (0.75f - 0.5f);
    float factor_day_to_night = (dayFactor - 0.5f) / (1.f - 0.5f);
    float factor_step = step(0.5f, factor_day_to_night);

    vec3 gradient_day_to_sunset = (factor_day_to_sunset * day_color) + ((1.f - factor_day_to_sunset) * sun_set_color);
    vec3 gradient_sunset_to_night = (factor_sunset_to_night * sun_set_color) + ((1.f - factor_sunset_to_night) * night_color);

    vec3 gradient_color = gradient_day_to_sunset * factor_step + (1.0 - factor_step) * gradient_sunset_to_night;
    // multiply it by the color
    gl_FragColor =  vec4(gradient_color, 1.0) * pixel * vec4(vec3(dayFactor),1.f);
}
