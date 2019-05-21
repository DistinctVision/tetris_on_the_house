in highp vec3 normal;

uniform highp vec4 mainColor;
uniform highp vec4 fallOffColor;

out highp vec4 color;

void main(void)
{
    highp float v = dot(vec3(0.0, 0.0, 1.0), normalize(normal));
    v *= v;
    //v = 1.0 - (1.0 - v) * (1.0 - v);
    color = mix(mainColor, fallOffColor, v);
}
