layout(location = 0) in highp vec4 vertex_position;
layout(location = 1) in highp vec2 vertex_textureCoord;

uniform highp mat4 matrixMVP;

uniform highp vec4 colorA;
uniform highp vec4 colorB;

uniform highp float plane_z;
uniform highp float plane_delta;
uniform highp float begin_z;

out highp vec4 v_color;

void main(void)
{
    highp float t = min(abs(vertex_position.z - plane_z) / plane_delta, 1.0);
    if (vertex_position.z < begin_z)
        t = 0.0;

    v_color = colorA * (1.0 - t) + colorB * t;

    vec4 delta = vec4(- vertex_textureCoord * t, 0.0, 0.0);
    gl_Position = matrixMVP * (vertex_position + delta);
}
