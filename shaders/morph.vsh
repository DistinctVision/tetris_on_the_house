layout(location = 0) in highp vec4 vertex_position;
layout(location = 1) in highp vec2 vertex_textureCoord;

uniform highp mat4 matrixMVP;

out highp vec4 view_position;
out highp vec2 texCoord;

void main(void)
{
    texCoord = vertex_textureCoord;
    view_position = matrixMVP * vertex_position;
    vec4 new_pos = vertex_position + vec4(vertex_textureCoord.xy, 0.0, 0.0);
    gl_Position = matrixMVP * new_pos;
}
