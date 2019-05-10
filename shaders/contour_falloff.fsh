in highp vec3 position;
in highp vec2 textureCoord;
in highp vec3 normal;

uniform highp vec4 fallOff_color;
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

    highp float v = 1.0 - max(dot(normalize(position), normalize(normal)), 0.0);
    v = 1.0 - (1.0 - v) * (1.0 - v);
    color = vec4(mix(fallOff_color.xyz, vec3(1.0), v), fallOff_color.w * v);
}
