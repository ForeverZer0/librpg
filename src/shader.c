
#include "game.h"
#include "glad.h"
#include "internal.h"
#include "rpg.h"

typedef struct RPGshader {
    GLuint program;
    void *user;
} RPGshader;

static RPGbool RPG_Shader_CreateShader(const char *source, GLenum type, GLuint *result) {
    GLuint shader = glCreateShader(type);
    GLint length  = (GLint) strlen(source);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        glDeleteShader(shader);
        *result = 0;
        return RPG_TRUE;
    }
    *result = shader;
    return RPG_FALSE;
}

RPG_RESULT RPG_Shader_Create(const char *vertSrc, const char *fragSrc, const char *geoSrc, RPGshader **shader) {
    RPG_RETURN_IF_NULL(vertSrc);
    RPG_RETURN_IF_NULL(fragSrc);

    GLuint program, vertex, fragment, geometry;
    if (RPG_Shader_CreateShader(vertSrc, GL_VERTEX_SHADER, &vertex)) {
        return RPG_ERR_SHADER_COMPILE;
    }
    if (RPG_Shader_CreateShader(fragSrc, GL_FRAGMENT_SHADER, &fragment)) {
        glDeleteShader(vertex);
        return RPG_ERR_SHADER_COMPILE;
    }
    if (geoSrc != NULL && RPG_Shader_CreateShader(geoSrc, GL_GEOMETRY_SHADER, &geometry)) {
        glDeleteShader(fragment);
        glDeleteShader(vertex);
        return RPG_ERR_SHADER_COMPILE;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    if (geometry > 0) {
        glAttachShader(program, geometry);
    }
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteShader(geometry);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        glDeleteProgram(program);
        return RPG_ERR_SHADER_LINK;
    }
    RPGshader *s = RPG_ALLOC(RPGshader);
    s->program   = program;
    s->user      = NULL;
    *shader      = s;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_Begin(RPGshader *shader) {
    RPG_RETURN_IF_NULL(shader);
    glUseProgram(shader->program);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_Finish(RPGshader *shader) {
    glUseProgram(RPG_GAME->shader.program);
    return RPG_NO_ERROR;
}