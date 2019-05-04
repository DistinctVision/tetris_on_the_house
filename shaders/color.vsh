layout(location = 0) in highp vec4 vertex_position;

uniform highp mat4 matrixMVP;

void main(void)
{
    gl_Position = matrixMVP * vertex_position;
}
