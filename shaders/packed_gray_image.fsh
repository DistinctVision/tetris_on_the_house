in highp vec2 textureCoord;

uniform sampler2D main_texture;
uniform highp vec4 steps;

out highp vec4 color;

highp float toGray(highp vec3 color)
{
    return (color.x + color.y + color.z) / 3.0;
}

void main(void)
{
    color = vec4(toGray(texture(main_texture, vec2(textureCoord.x + steps.x, textureCoord.y)).xyz),
                 toGray(texture(main_texture, vec2(textureCoord.x + steps.y, textureCoord.y)).xyz),
                 toGray(texture(main_texture, vec2(textureCoord.x + steps.z, textureCoord.y)).xyz),
                 toGray(texture(main_texture, vec2(textureCoord.x + steps.w, textureCoord.y)).xyz));
}
