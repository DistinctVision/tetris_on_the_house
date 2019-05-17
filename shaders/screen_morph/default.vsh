layout(location = 0) in highp vec4 vertex_position;

uniform highp mat4 matrixMVP;

out highp vec4 view_position;

void main(void)
{
    gl_Position = view_position = matrixMVP * vertex_position;
}
