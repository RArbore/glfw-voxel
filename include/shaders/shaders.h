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

    struct dir_light_s {
        vec3 direction;
        vec3 ambient_color;
        vec3 diffuse_color;
        vec3 specular_color;
    };

    struct point_light_s {
        vec3 position;
        vec3 ambient_color;
        vec3 diffuse_color;
        vec3 specular_color;
        vec3 attenuation;
    };

    #define NUM_LIGHTS 1

    in vec3 position1;
    in vec3 normal1;
    in vec2 texcoord1;

    out vec4 color;

    uniform dir_light_s dir_light;
    uniform point_light_s point_light[NUM_LIGHTS];

    uniform vec3 camera_pos;

    uniform sampler2D tex_atlas;

    vec3 calc_dir_light_contrib(dir_light_s light, vec3 normal, vec3 camera_dir) {
        vec3 light_dir = normalize(-light.direction);
        float diff = max(dot(normal, light_dir), 0.0);
        vec3 reflect_dir = reflect(-light_dir, normal);
        float spec = pow(max(dot(camera_dir, reflect_dir), 0.0), 64);
        vec3 ambient = light.ambient_color;
        vec3 diffuse = light.diffuse_color * diff;
        vec3 specular = light.specular_color * spec;
        return ambient + diffuse + specular;
    }

    vec3 calc_point_light_contrib(point_light_s light, vec3 normal, vec3 pos, vec3 camera_dir) {
        vec3 light_dir = normalize(light.position - pos);
        float diff = max(dot(normal, light_dir), 0.0);
        vec3 reflect_dir = reflect(-light_dir, normal);
        float spec = pow(max(dot(camera_dir, reflect_dir), 0.0), 64);
        float distance = length(light.position - pos);
        float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance * distance);
        vec3 ambient = light.ambient_color;
        vec3 diffuse = light.diffuse_color * diff;
        vec3 specular = light.specular_color * spec;
        return (ambient + diffuse + specular) * attenuation;
    }

    void main() {
        vec3 norm = normalize(normal1);
        vec3 camera_dir = normalize(camera_pos - position1);
        vec3 result = calc_dir_light_contrib(dir_light, norm, camera_dir);
        int i;
        for (i = 0; i < NUM_LIGHTS; i++) {
            result += calc_point_light_contrib(point_light[i], norm, position1, camera_dir);
        }
        float distance = length(camera_pos - position1);
        float fog = 1.0 - 1.0/(distance*distance*0.0001 + 1.0);
        color = vec4(result * vec3(texture(tex_atlas, texcoord1)) * (1.0 - fog) + vec3(0.4, 0.4, 0.4) * fog, 1.0);
    }

)glsl";

#endif // __SHADERS_H_
