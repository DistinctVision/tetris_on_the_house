layout(location = 0) in highp vec4 vertex_position;
layout(location = 1) in highp vec2 vertex_textureCoord;
layout(location = 2) in highp vec3 vertex_normal;

uniform highp mat4 matrixMVP;
uniform highp mat4 matrixMV;

out highp vec3 position;
out highp vec2 textureCoord;
out highp vec3 normal;

void main(void)
{
    position = (matrixMV * vertex_position).xyz;
    textureCoord = vertex_textureCoord;
    normal = normalize((matrixMV * vec4(vertex_normal, 0.0)).xyz);
    gl_Position = matrixMVP * vertex_position;
}
