#ifndef __SHADERS_H_
#define __SHADERS_H_

const char* vertex_source = R"glsl(

    #version 330 core

    in vec3 position;
    in vec3 normal;
    in vec2 texcoord;

    out vec3 position1;
    out vec3 normal1;
    out vec2 texcoord1;

    uniform mat4 world_mat;
    uniform mat4 view_mat;
    uniform mat4 proj_mat;

    void main() {
        position1 = vec3(world_mat * vec4(position, 1.0));
        normal1 = normal;
        texcoord1 = texcoord;
        gl_Position = proj_mat * view_mat * vec4(position1, 1.0);
    }

)glsl";

const char* fragment_source = R"glsl(

    #version 330 core

    in vec3 position1;
    in vec3 normal1;
    in vec2 texcoord1;

    out vec4 color;

    uniform vec3 light_pos;
    uniform vec3 light_color;
    uniform sampler2D tex_atlas;

    void main() {
        float ambient_strength = 0.3;
        vec3 norm = normalize(normal1);
        vec3 light_dir = normalize(light_pos - position1);
        float diff = max(dot(norm, light_dir), 0.0);
        vec3 diffuse = diff * light_color;
        color = texture(tex_atlas, texcoord1) * vec4(light_color * ambient_strength + diffuse * (1.0 - ambient_strength), 1.0);
    }

)glsl";

#endif // __SHADERS_H_
