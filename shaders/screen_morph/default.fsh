in highp vec4 view_position;
uniform sampler2D screen_texture;
uniform highp mat4 matrixView2FrameUV;

uniform highp vec3 color_a;
uniform highp vec3 color_b;

out highp vec4 color;

highp vec3 v_project(highp vec4 v)
{
    return (abs(v.w) > 1e-3) ? v.xyz / v.w : vec3(0.0);
}

highp vec3 rgb2hsv(highp vec3 c)
{
    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    highp float d = q.x - min(q.w, q.y);
    highp float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

highp vec3 hsv2rgb(highp vec3 c)
{
    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

highp vec3 changeColor(highp vec3 c)
{
    highp vec3 c1 = rgb2hsv(c);
    c1 = vec3(fract(c1.x * color_a.x + color_b.x),
              clamp(c1.y * color_a.y + color_b.y, 0.0, 1.0),
              clamp(c1.z * color_a.z + color_b.z, 0.0, 1.0));
    return hsv2rgb(c1);
}

void main(void)
{
    highp vec2 uv = v_project(matrixView2FrameUV * view_position).xy;
    color = vec4(changeColor(texture(screen_texture, uv).bgr), 1.0);
}
