#version 410 core

out vec4 color;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_uv;

uniform sampler2D tex_base_color;

vec3 reflect(vec3 a, vec3 n)
{
    float dot2 = dot(a, n) * 2;
    vec3 v = n*dot2;
    return a-v;
}

void main(){
    vec3 pixel_light_vector = vec3(0, 0, -1);
    float lambert = clamp(dot(normalize(pixel_light_vector), normalize(v_normal)), 0, 1);

    vec3 reflected_vec = normalize(reflect(pixel_light_vector, v_normal));
    float specular = clamp(dot(reflected_vec, v_position), 0, 1);

    vec3 metal_color = vec3(1, 1, 1);
    vec3 specular_color = metal_color * pow(specular, 1);

    vec3 final_color= texture(tex_base_color, vec2(v_uv.x, 1 - v_uv.y)).xyz;

    color = vec4((final_color * lambert)+(specular_color), 1);
}