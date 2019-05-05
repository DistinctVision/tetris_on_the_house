in highp vec2 textureCoord;

uniform sampler2D main_texture;
uniform highp float step_x;

out highp vec4 color;

highp float toGray(highp vec3 color)
{
    return (color.x + color.y + color.z) / 3.0;
}

void main(void)
{
    color = vec4(toGray(texture(main_texture, textureCoord).xyz),
                 toGray(texture(main_texture, vec2(textureCoord.x + step_x, textureCoord.y)).xyz),
                 toGray(texture(main_texture, vec2(textureCoord.x + step_x * 2.0, textureCoord.y)).xyz),
                 toGray(texture(main_texture, vec2(textureCoord.x + step_x * 3.0, textureCoord.y)).xyz));
}
