//in highp vec3 position;
in highp vec2 textureCoord;
in highp vec3 normal;

uniform highp vec4 mainColor;
uniform highp float border_size;
uniform highp vec4 border_color;

out highp vec4 color;

void main(void)
{
    if (min(min(abs(textureCoord.x - 1.0), abs(textureCoord.y - 1.0)),
            min(abs(textureCoord.x), abs(textureCoord.y))) < border_size)
    {
        color = border_color;
        return;
    }

    highp float v = max(dot(vec3(0.0, 0.0, 1.0), normalize(normal)), 0.0);
    v *= v;
    //v = 1.0 - (1.0 - v) * (1.0 - v);
    color = vec4(mix(mainColor.xyz, vec3(1.0), v), mainColor.w * v);
}
