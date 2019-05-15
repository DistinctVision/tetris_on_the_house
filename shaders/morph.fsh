in highp vec4 view_position;
in highp vec2 texCoord;

uniform sampler2D screen_texture;
uniform highp mat4 matrixView2FrameUV;

out highp vec4 color;

highp vec3 v_project(highp vec4 v)
{
    return (abs(v.w) > 1e-3) ? v.xyz / v.w : vec3(0.0);
}

highp vec3 to_gray(highp vec3 v)
{
    return v.zyx;
    highp float c = (v.x + v.y + v.z) / 3.0;
    return vec3(c, c, c);
}

void main(void)
{
    highp vec2 uv = v_project(matrixView2FrameUV * view_position).xy;
    color = vec4(to_gray(texture(screen_texture, uv).xyz), 1.0);
}
