layout(location = 0) in highp vec4 vertex_position;

uniform highp mat4 matrixMVP;
uniform highp mat4 matrixMVP_transform;

out highp vec4 view_position;

void main(void)
{
    view_position = matrixMVP * vertex_position;
    gl_Position = matrixMVP_transform * vertex_position;
}
