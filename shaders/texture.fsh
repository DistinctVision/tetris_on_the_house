in highp vec2 textureCoord;

uniform sampler2D main_texture;

out highp vec4 color;

void main(void)
{
    color = texture(main_texture, textureCoord);
}
