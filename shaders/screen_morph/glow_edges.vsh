layout(location = 0) in highp vec4 vertex_position;
layout(location = 1) in highp vec2 vertex_textureCoord;

uniform highp mat4 matrixMVP;

out highp vec4 view_position;
out highp vec2 textureCoord;

void main(void)
{
    textureCoord = vertex_textureCoord;
    gl_Position = view_position = matrixMVP * vertex_position;
}
