varying highp vec4 textureCoord;

uniform sampler2D main_texture;

out highp vec4 color;

void main(void)
{
    color = texture2D(main_texture, textureCoord);
}
