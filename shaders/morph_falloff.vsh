layout(location = 0) in highp vec4 vertex_position;
layout(location = 1) in highp vec3 vertex_normal;

layout(location = 2) in highp vec4 vertex_morph_position;
layout(location = 3) in highp vec3 vertex_morph_normal;

uniform highp mat4 matrixMVP;
uniform highp mat4 matrixMV;

uniform highp float time;

out highp vec3 normal;

void main(void)
{
    //normal = normalize((matrixMV * vec4(vertex_normal, 0.0)).xyz);
    //gl_Position = matrixMVP * vertex_position;
    normal = normalize((matrixMV * vec4(mix(vertex_normal, vertex_morph_normal, time), 0.0)).xyz);
    gl_Position = matrixMVP * mix(vertex_position, vertex_morph_position, time);
}
