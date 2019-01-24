#version 410 core
const float PI = 3.14159265358979323846264338327950288;

layout(location = 0) in vec3 input_position;
layout(location = 1) in vec3 input_normal;
layout(location = 2) in vec2 input_uv;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_uv;

uniform vec3 position;
uniform vec3 rotation;
uniform vec3 scale;

vec3 vec3_rotx(vec3 a, float r)
{
    vec3 v;
    v.x = a.x;
    v.y = cos(r)*a.y - sin(r)*a.z;
    v.z = sin(r)*a.y + cos(r)*a.z;
    return v;
}

vec3 vec3_roty(vec3 a, float r)
{
    vec3 v;
    v.x = cos(r) * a.x - sin(r) * a.z;
    v.y = a.y;
    v.z = sin(r) * a.x + cos(r) * a.z;
    return v;
}

vec3 vec3_rotz(vec3 a, float r)
{
    vec3 v;
    v.x = cos(r) * a.x - sin(r) * a.y;
    v.y = sin(r) * a.x + cos(r) * a.y;
    v.z = a.z;
    return v;
}

float linear_convert(float value, float old_min, float old_max, float new_min, float new_max){
    float gradient = (value - old_min) / (old_max - old_min);
    return new_min + gradient * (new_max - new_min);
}

vec3 perspective_convert(vec3 a)
{   
    a.z += 5;

    float fov = 60.0;
    a.x /= (a.z * tan(fov * 0.5 * (PI/180)))*(1440/720);
    a.y /= (a.z * tan(fov * 0.5 * (PI/180)));
    a.z = ((a.z - 1)/(1000-1)) -1; // questo mi porta nel range ( - 1 + 1)
    return a;
}

void main(){
    // if(gl_VertexID == 0){
    //     gl_Position = vec4(0, 0, 0, 1);
    //     vertex_color = vec3(1, 1, 0);
    // }
    // if(gl_VertexID == 1){
    //     gl_Position = vec4(0.5, 0, 0, 1);
    //     vertex_color = vec3(0, 0, 1);
    // }
    // if(gl_VertexID == 2){
    //     gl_Position = vec4(0.5, -0.5, 0, 1);
    //     vertex_color = vec3(0, 1, 0);
    // }
    vec3 final_pos = vec3(input_position.y, input_position.z, input_position.x);
    final_pos = vec3_rotx(final_pos, rotation.x);
    final_pos = vec3_roty(final_pos, rotation.y);
    final_pos = vec3_rotz(final_pos, rotation.z);
    final_pos = perspective_convert(final_pos * scale);
    final_pos += position;

    // vec3 position = vec3_roty(vec3(input_position.y, input_position.z, input_position.x), rotation.y);
    // final_pos.x = linear_convert(final_pos.x, -200, 200, -1, 1);
    // final_pos.y = linear_convert(final_pos.y, -200, 200, -1, 1);
    // final_pos.z = linear_convert(final_pos.z, -200, 200, -1, 1);
    // position = perspective_convert(pos * 0.02);
    // pos.x += x;
    // pos.y += y;
    v_position = final_pos;
    gl_Position = vec4(final_pos, 1);

    v_normal = input_normal;
    v_normal = vec3(v_normal.y, v_normal.z, v_normal.x);
    v_normal = vec3_rotx(v_normal, rotation.x);
    v_normal = vec3_roty(v_normal, rotation.y);
    v_normal = vec3_rotz(v_normal, rotation.z);

    v_uv = input_uv;
}