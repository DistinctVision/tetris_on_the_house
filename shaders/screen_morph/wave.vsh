layout(location = 0) in highp vec4 vertex_position;
layout(location = 1) in highp vec2 vertex_textureCoord;

uniform highp mat4 matrixMVP;

uniform highp float wave_threshold;
uniform highp vec3 wave_origin;
uniform highp float wave_time;
uniform highp float wave_timeScale;
uniform highp float wave_distanceStep;
uniform highp float wave_distanceStepScale;
uniform highp float wave_scale;

out highp vec4 view_position;

const highp float M_PI = 3.14159265;

void main(void)
{
    float k = min(min(vertex_textureCoord.x, vertex_textureCoord.y),
                  min(1.0 - vertex_textureCoord.x, 1.0 - vertex_textureCoord.y));
    vec3 wave_dir = (vertex_position.xyz - wave_origin);
    float distance = length(wave_dir);
    wave_dir /= max(distance, 0.01);
    float d = clamp((fract(distance / wave_distanceStep - wave_time * wave_timeScale) * wave_distanceStepScale), 0.0, 1.0);
    vec4 delta = vec4((sin(d * (2.0 * M_PI)) * wave_scale) * wave_dir, 0.0);

    view_position = matrixMVP * (vertex_position + delta);
    gl_Position = matrixMVP * vertex_position;
}
