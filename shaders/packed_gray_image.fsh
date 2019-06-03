uniform sampler2D main_texture;

uniform int offsets[4];

highp float toGray(highp vec3 color)
{
    return (color.x);
}

out highp vec4 color;

void main(void)
{
    ivec2 c = ivec2(int(gl_FragCoord.x) * 4, int(gl_FragCoord.y));
    color = vec4(toGray(texelFetch(main_texture, ivec2(c.x + offsets[0], c.y), 0).xyz),
                 toGray(texelFetch(main_texture, ivec2(c.x + offsets[1], c.y), 0).xyz),
                 toGray(texelFetch(main_texture, ivec2(c.x + offsets[2], c.y), 0).xyz),
                 toGray(texelFetch(main_texture, ivec2(c.x + offsets[3], c.y), 0).xyz));
}
