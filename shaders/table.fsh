in highp vec2 textureCoord;

uniform highp vec4 tableColor;
uniform highp int tableSize;
uniform highp vec4 tableEdgeColor;
uniform highp float tableEdgeSize;
uniform highp float level;

out highp vec4 color;

void main(void)
{
    highp int level_i = int(level);

    highp vec2 p = vec2(textureCoord.x, textureCoord.y * float(tableSize));
    mediump int l = int(floor(p.y));
    p.y -= float(l);
    highp float t = min(min(min(p.x, 1.0 - p.x), min(p.y, 1.0 - p.y)) / tableEdgeSize, 1.0);
    if (l > level_i)
    {
        color = tableEdgeColor;
        return;
    }
    else if (l == level_i)
    {
        t *= (level - float(level_i));
    }
    color = mix(tableEdgeColor, tableColor, t);
}
