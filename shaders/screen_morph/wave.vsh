layout(location = 0) in highp vec4 vertex_position;
layout(location = 1) in highp vec2 vertex_textureCoord;

uniform highp mat4 matrixMVP;
uniform highp float wave_time;
uniform highp float wave_timeScale;
uniform highp float wave_scale;
uniform highp float wave_threshold;

out highp vec4 view_position;

const highp float M_PI = 3.14159265;

void main(void)
{
    float k = min(min(vertex_textureCoord.x, vertex_textureCoord.y),
                  min(1.0 - vertex_textureCoord.x, 1.0 - vertex_textureCoord.y));
    vec4 delta = vec4(sin((wave_time + vertex_textureCoord.x) * 3.14159265 * wave_timeScale) * wave_scale,
                      0.0, 0.0, 0.0);

    view_position = matrixMVP * (vertex_position + delta * k);
    gl_Position = matrixMVP * vertex_position;
}
