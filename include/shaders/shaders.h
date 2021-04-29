#ifndef __SHADERS_H_
#define __SHADERS_H_

const char* vertex_source = R"glsl(

    #version 150 core

    in vec2 position;
    in vec3 color;
    in vec2 texcoord;

    out vec3 color1;
    out vec2 texcoord1;

    void main() {
        color1 = color;
        texcoord1 = texcoord;
        gl_Position = vec4(position, 0.0, 1.0);
    }

)glsl";

const char* fragment_source = R"glsl(

    #version 150 core

    in vec3 color1;
    in vec2 texcoord1;

    out vec4 color2;

    uniform sampler2D tex_atlas;

    void main() {
        color2 = texture(tex_atlas, texcoord1) * vec4(color1, 1.0);
    }

)glsl";

#endif // __SHADERS_H_
