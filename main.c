#define SDL_MAIN_HANDLED
#define STB_IMAGE_IMPLEMENTATION
#include <glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "stb_image.h"
#include "aiv_math.h"
#include "libfbxc.h"

typedef int(*adder_signature)(int a, int b);

typedef struct mesh{
    Vector3_t position;
    Vector3_t rotation;
    Vector3_t scale;
    fbxc_scene_t* scene;
    GLuint vao;
    GLuint vbo[3];
    GLuint texture;
    unsigned int mesh_id;
    GLint position_uniform, rotation_uniform, texture_uniform, scale_uniform;
} mesh_t;

mesh_t* new_mesh(const char* mesh_name, const char* texture_name, unsigned int* mesh_id, GLuint program){
    mesh_t* to_return;
    to_return = malloc(sizeof(mesh_t));
    memset(to_return, 0, sizeof(mesh_t));
    if(!to_return){
        SDL_Log("Couldn't allocate mesh memory.\n");
        return NULL;
    }

    to_return->scene = fbxc_parse_file(mesh_name);
    if(!to_return->scene){
        SDL_Log("Couldn't load mesh.\n");
        return NULL;
    }
    
    int w, h, comp;
    unsigned char *pixels = stbi_load(texture_name, &w, &h, &comp, 4);
    if(!pixels){
        SDL_Log("Couldn't load texture.\n");
        return NULL;
    }
    to_return->scale = Vector3_new(1, 1, 1);

    to_return->vao = *mesh_id;

    glGenVertexArrays(1, &to_return->vao);
    glBindVertexArray(to_return->vao);

    glGenBuffers(3, to_return->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, to_return->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, to_return->scene->vertices_len * sizeof(float), to_return->scene->vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, to_return->vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, to_return->scene->normals_len * sizeof(float), to_return->scene->normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, to_return->vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, to_return->scene->uvs_len * sizeof(float), to_return->scene->uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint texture = *mesh_id;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    to_return->position_uniform = glGetUniformLocation(program, "position");
    to_return->rotation_uniform = glGetUniformLocation(program, "rotation");
    to_return->scale_uniform = glGetUniformLocation(program, "scale");
    to_return->texture_uniform = glGetUniformLocation(program,"tex_base_color");

    to_return->mesh_id = *mesh_id;
    *mesh_id += 1;
    free(pixels);
    return to_return;
}

void draw_mesh(mesh_t* to_draw){
    glBindVertexArray(to_draw->vao);
    glBindTexture(GL_TEXTURE_2D, to_draw->vao);

    glUniform3f(to_draw->position_uniform, to_draw->position.x, to_draw->position.y, to_draw->position.z);
    glUniform3f(to_draw->rotation_uniform, to_draw->rotation.x, to_draw->rotation.y, to_draw->rotation.z);
    glUniform3f(to_draw->scale_uniform, to_draw->scale.x, to_draw->scale.y, to_draw->scale.z);
    glUniform1i(to_draw->texture_uniform, 0);

    glDrawArrays(GL_TRIANGLES, 0, to_draw->scene->vertices_len / 3);
}

GLuint compile_shader(GLenum shader_type, const char* filename){
    SDL_RWops* rw = SDL_RWFromFile(filename, "rb");
    if(!rw){
        SDL_Log("unable to open file");
        exit(1);
    }

    size_t file_len = SDL_RWsize(rw);
    char* source = SDL_malloc(file_len + 1);
    if(!source){
        SDL_Log("couldn't allocate source.");
        exit(1);
    }
    if(SDL_RWread(rw, source, 1, file_len) != file_len){
        SDL_Log("couldn't read file.");
        exit(1);
    }

    source[file_len] = 0;

    SDL_RWclose(rw);

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const GLchar**)&source, (GLint*)&file_len);
    glCompileShader(shader);

    SDL_free(source);

    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status == GL_FALSE){
        GLint log_size;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        char* error_log = SDL_malloc(log_size +1);
        if(!error_log){
            SDL_Log("unable to allocate memory for log");
            exit(1);
        }
        glGetShaderInfoLog(shader, log_size, &log_size, error_log);
        SDL_Log("shader compile error %s", error_log);
        SDL_free(error_log);
        exit(1);
    }

    return shader;
}

int main(int argc, char **argv){
    unsigned int mesh_id = 0;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1440, 720, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

    SDL_GL_CreateContext(window);

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    SDL_Log("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
    SDL_Log("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
    SDL_Log("GL_VERSION: %s\n", glGetString(GL_VERSION));
    SDL_Log("GL_SHADING_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    SDL_GL_SetSwapInterval(0);

    glClearColor(0, 0, 0, 1);

    GLuint program = glCreateProgram();

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, "vertex.glsl");
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glUseProgram(program);

    mesh_t* mesh0 = new_mesh("StaticMesh.FBX", "Texture.TGA", &mesh_id, program);
    mesh0->position.x = -0.7;
    mesh0->position.y = -0.9;
    mesh0->scale = Vector3_new(0.02, 0.02, 0.02);

    mesh_t* mesh1 = new_mesh("TemplateFloor.FBX", "Random.TGA", &mesh_id, program);
    mesh1->position.x = 0.7;
    mesh1->position.y = -0.9;
    mesh1->scale = Vector3_new(0.002, 0.001, 0.001);

    if (!window)
        return -1;

    for (;;)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return 0;
            if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_RIGHT){
                    mesh1->position.x += 0.1;
                }
                if(event.key.keysym.sym == SDLK_LEFT){
                    mesh1->position.x -= 0.1;
                }
                if(event.key.keysym.sym == SDLK_UP){
                    mesh1->position.y += 0.1;
                }
                if(event.key.keysym.sym == SDLK_DOWN){
                    mesh1->position.y -= 0.1;
                }
            }
        }
        mesh0->rotation.y += 0.001;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw_mesh(mesh0);
        draw_mesh(mesh1);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}