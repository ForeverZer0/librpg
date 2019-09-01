
#include "internal.h"
#include <stdio.h>

#define RPG_SHADER_ENSURE_ACTIVE(shader)                                                                                                   \
    if (shader == NULL)                                                                                                                    \
        return RPG_ERR_INVALID_POINTER;                                                                                                    \
    GLint _id;                                                                                                                             \
    glGetIntegerv(GL_CURRENT_PROGRAM, &_id);                                                                                               \
    if (_id != shader->program)                                                                                                            \
        return RPG_ERR_SHADER_NOT_ACTIVE;

static const char *RPG_Shader_GetTransitionSource(RPG_TRANSITION_TYPE type) {
    switch (type) {
        case RPG_TRANSITION_TYPE_GLITCH_DISPLACE: return RPG_TRANSITION_GLITCH_DISPLACE;
        case RPG_TRANSITION_TYPE_DIRECTIONAL_WARP: return RPG_TRANSITION_DIRECTIONAL_WARP;
        case RPG_TRANSITION_TYPE_LUMINANCE_MELT: return RPG_TRANSITION_LUMINANCE_MELT;
        case RPG_TRANSITION_TYPE_PINWHEEL: return RPG_TRANSITION_PINWHEEL;
        case RPG_TRANSITION_TYPE_CIRCLE: return RPG_TRANSITION_CIRCLE;
        case RPG_TRANSITION_TYPE_LUMA: return RPG_TRANSITION_LUMA;
        case RPG_TRANSITION_TYPE_RIPPLE: return RPG_TRANSITION_RIPPLE;
        case RPG_TRANSITION_TYPE_CURTAIN: return RPG_TRANSITION_CURTAIN;
        case RPG_TRANSITION_TYPE_DIRECTIONAL_WIPE: return RPG_TRANSITION_DIRECTIONAL_WIPE;
        case RPG_TRANSITION_TYPE_WATERDROP: return RPG_TRANSITION_WATERDROP;
        case RPG_TRANSITION_TYPE_GRID_FLIP: return RPG_TRANSITION_GRID_FLIP;
        case RPG_TRANSITION_TYPE_BURN: return RPG_TRANSITION_BURN;
        case RPG_TRANSITION_TYPE_COLOR_PHASE: return RPG_TRANSITION_COLOR_PHASE;
        case RPG_TRANSITION_TYPE_SWIRL: return RPG_TRANSITION_SWIRL;
        case RPG_TRANSITION_TYPE_HEART: return RPG_TRANSITION_HEART;
        case RPG_TRANSITION_TYPE_HEXAGONALIZE: return RPG_TRANSITION_HEXAGONALIZE;
        case RPG_TRANSITION_TYPE_SQUARES_WIRE: return RPG_TRANSITION_SQUARES_WIRE;
        case RPG_TRANSITION_TYPE_WINDOWBLINDS: return RPG_TRANSITION_WINDOWBLINDS;
        case RPG_TRANSITION_TYPE_STATIC: return RPG_TRANSITION_STATIC;
        case RPG_TRANSITION_TYPE_DISPLACEMENT: return RPG_TRANSITION_DISPLACEMENT;
        case RPG_TRANSITION_TYPE_BOW_TIE_V: return RPG_TRANSITION_BOW_TIE_V;
        case RPG_TRANSITION_TYPE_PARAMETRICS: return RPG_TRANSITION_PARAMETRICS;
        case RPG_TRANSITION_TYPE_WIPE_LEFT: return RPG_TRANSITION_WIPE_LEFT;
        case RPG_TRANSITION_TYPE_DIRECTIONAL: return RPG_TRANSITION_DIRECTIONAL;
        case RPG_TRANSITION_TYPE_WIPE_DOWN: return RPG_TRANSITION_WIPE_DOWN;
        case RPG_TRANSITION_TYPE_MORPH: return RPG_TRANSITION_MORPH;
        case RPG_TRANSITION_TYPE_FADE_GRAYSCALE: return RPG_TRANSITION_FADE_GRAYSCALE;
        case RPG_TRANSITION_TYPE_ROTATE_SCALE_FADE: return RPG_TRANSITION_ROTATE_SCALE_FADE;
        case RPG_TRANSITION_TYPE_WIPE_UP: return RPG_TRANSITION_WIPE_UP;
        case RPG_TRANSITION_TYPE_CROSS_ZOOM: return RPG_TRANSITION_CROSS_ZOOM;
        case RPG_TRANSITION_TYPE_MOSAIC: return RPG_TRANSITION_MOSAIC;
        case RPG_TRANSITION_TYPE_PIXELIZE: return RPG_TRANSITION_PIXELIZE;
        case RPG_TRANSITION_TYPE_SQUEEZE: return RPG_TRANSITION_SQUEEZE;
        case RPG_TRANSITION_TYPE_WINDOW_SLICE: return RPG_TRANSITION_WINDOW_SLICE;
        case RPG_TRANSITION_TYPE_RADIAL: return RPG_TRANSITION_RADIAL;
        case RPG_TRANSITION_TYPE_FADE_COLOR: return RPG_TRANSITION_FADE_COLOR;
        case RPG_TRANSITION_TYPE_SWAP: return RPG_TRANSITION_SWAP;
        case RPG_TRANSITION_TYPE_KALEIDOSCOPE: return RPG_TRANSITION_KALEIDOSCOPE;
        case RPG_TRANSITION_TYPE_ANGULAR: return RPG_TRANSITION_ANGULAR;
        case RPG_TRANSITION_TYPE_DOORWAY: return RPG_TRANSITION_DOORWAY;
        case RPG_TRANSITION_TYPE_MULTIPLY_BLEND: return RPG_TRANSITION_MULTIPLY_BLEND;
        case RPG_TRANSITION_TYPE_LINEAR_BLUR: return RPG_TRANSITION_LINEAR_BLUR;
        case RPG_TRANSITION_TYPE_WIPE_RIGHT: return RPG_TRANSITION_WIPE_RIGHT;
        case RPG_TRANSITION_TYPE_CROSS_WARP: return RPG_TRANSITION_CROSS_WARP;
        case RPG_TRANSITION_TYPE_GLITCH_MEMORIES: return RPG_TRANSITION_GLITCH_MEMORIES;
        case RPG_TRANSITION_TYPE_SQUARES: return RPG_TRANSITION_SQUARES;
        case RPG_TRANSITION_TYPE_SIMPLE_ZOOM: return RPG_TRANSITION_SIMPLE_ZOOM;
        case RPG_TRANSITION_TYPE_PERLIN: return RPG_TRANSITION_PERLIN;
        case RPG_TRANSITION_TYPE_FLYEYE: return RPG_TRANSITION_FLYEYE;
        case RPG_TRANSITION_TYPE_CUBE: return RPG_TRANSITION_CUBE;
        case RPG_TRANSITION_TYPE_BURNOUT: return RPG_TRANSITION_BURNOUT;
        case RPG_TRANSITION_TYPE_PAGE_CURL: return RPG_TRANSITION_PAGE_CURL;
        case RPG_TRANSITION_TYPE_FADE: return RPG_TRANSITION_FADE;
        case RPG_TRANSITION_TYPE_FILM_BURN: return RPG_TRANSITION_FILM_BURN;
        case RPG_TRANSITION_TYPE_DOOM: return RPG_TRANSITION_DOOM;
        case RPG_TRANSITION_TYPE_CROSSHATCH: return RPG_TRANSITION_CROSSHATCH;
        case RPG_TRANSITION_TYPE_BOUNCE: return RPG_TRANSITION_BOUNCE;
        case RPG_TRANSITION_TYPE_CIRCLE_OPEN: return RPG_TRANSITION_CIRCLE_OPEN;
        case RPG_TRANSITION_TYPE_BOW_TIE_H: return RPG_TRANSITION_BOW_TIE_H;
        case RPG_TRANSITION_TYPE_DREAMY: return RPG_TRANSITION_DREAMY;
        case RPG_TRANSITION_TYPE_POLAR: return RPG_TRANSITION_POLAR;
        case RPG_TRANSITION_TYPE_COLOR_DISTANCE: return RPG_TRANSITION_COLOR_DISTANCE;
        case RPG_TRANSITION_TYPE_WIND: return RPG_TRANSITION_WIND;
        default: return NULL;
    }
}

static RPGbool RPG_Shader_CreateShader(const char *source, GLenum type, GLuint *result) {
    GLuint shader = glCreateShader(type);
    GLint length  = (GLint) strlen(source) + 1;
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        *result = 0;
        #ifdef RPG_DEBUG
        char msg[512];
        glGetShaderInfoLog(shader, 512, NULL, msg);
        fprintf(stderr, "%s", msg);
        #endif
        glDeleteShader(shader);
        return RPG_TRUE;
    }
    *result = shader;
    return RPG_FALSE;
}

RPG_RESULT RPG_Shader_Create(const char *vertSrc, const char *fragSrc, const char *geoSrc, RPGshader **shader) {
    RPG_RETURN_IF_NULL(shader);
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

RPG_RESULT RPG_Shader_CreateTransition(RPG_TRANSITION_TYPE type, RPGshader **shader) {
    if (type < 0 || type > RPG_TRANSITION_TYPE_LAST) {
        *shader = NULL;
        return RPG_ERR_OUT_OF_RANGE;
    }

    const char *source = RPG_Shader_GetTransitionSource(type);
    RPGsize size       = strlen(source) + strlen(RPG_TRANSITION_BASE_FRAGMENT) - 1;
    char buffer[size];
    sprintf(buffer, RPG_TRANSITION_BASE_FRAGMENT, source);

    return RPG_Shader_Create(RPG_TRANSITION_BASE_VERTEX, buffer, NULL, shader);
}

RPG_RESULT RPG_Shader_Free(RPGshader *shader) {
    if (shader != NULL) {
        glDeleteProgram(shader->program);
        RPG_FREE(shader);
    }
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

RPG_RESULT RPG_Shader_GetUniformf(RPGshader *shader, RPGint location, RPGfloat *v) {
    RPG_RETURN_IF_NULL(shader);
    glGetUniformfv(shader->program, location, v);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_GetUniformi(RPGshader *shader, RPGint location, RPGint *v) {
    RPG_RETURN_IF_NULL(shader);
    glGetUniformiv(shader->program, location, v);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform1f(RPGshader *shader, RPGint location, RPGfloat v1) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform1f(location, v1);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform2f(RPGshader *shader, RPGint location, RPGfloat v1, RPGfloat v2) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform2f(location, v1, v2);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform3f(RPGshader *shader, RPGint location, RPGfloat v1, RPGfloat v2, RPGfloat v3) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform3f(location, v1, v2, v3);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform4f(RPGshader *shader, RPGint location, RPGfloat v1, RPGfloat v2, RPGfloat v3, RPGfloat v4) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform4f(location, v1, v2, v3, v4);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform1i(RPGshader *shader, RPGint location, RPGint v1) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform1i(location, v1);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform2i(RPGshader *shader, RPGint location, RPGint v1, RPGint v2) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform2i(location, v1, v2);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform3i(RPGshader *shader, RPGint location, RPGint v1, RPGint v2, RPGint v3) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform3i(location, v1, v2, v3);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniform4i(RPGshader *shader, RPGint location, RPGint v1, RPGint v2, RPGint v3, RPGint v4) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform4i(location, v1, v2, v3, v4);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniformVec2(RPGshader *shader, RPGint location, RPGvec2 *vec) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    if (vec != NULL) {
        glUniform2f(location, vec->x, vec->y);
    } else {
        glUniform2f(location, 0.0f, 0.0f);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniformVec3(RPGshader *shader, RPGint location, RPGvec3 *vec) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    if (vec != NULL) {
        glUniform3f(location, vec->x, vec->y, vec->z);
    } else {
        glUniform3f(location, 0.0f, 0.0f, 0.0f);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniformVec4(RPGshader *shader, RPGint location, RPGvec4 *vec) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    if (vec != NULL) {
        glUniform4f(location, vec->x, vec->y, vec->z, vec->w);
    } else {
        glUniform4f(location, 0.0f, 0.0f, 0.0f, 0.0f);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniformMat3(RPGshader *shader, RPGint location, RPGmat3 *mat) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    if (mat != NULL) {
        glUniformMatrix3fv(location, 1, GL_FALSE, (GLfloat *) mat);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUniformMat4(RPGshader *shader, RPGint location, RPGmat4 *mat) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    if (mat != NULL) {
        glUniformMatrix4fv(location, 1, GL_FALSE, (GLfloat *) mat);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_BindImage(RPGshader *shader, RPGint location, RPGint unit, RPGimage *image) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    glUniform1i(location, unit);
    RPG_Drawing_BindTexture(image != NULL ? image->texture : 0, GL_TEXTURE0 + unit);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_UnbindImage(RPGshader *shader, RPGint unit) {
    RPG_SHADER_ENSURE_ACTIVE(shader);
    RPG_Drawing_BindTexture(0, GL_TEXTURE0 + unit);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_GetUniformLocation(RPGshader *shader, const char *name, RPGint *location) {
    RPG_RETURN_IF_NULL(shader);
    if (location != NULL) {
        *location = glGetUniformLocation(shader->program, name);
    } 
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_GetIsActive(RPGshader *shader, RPGbool *active) {
    RPG_RETURN_IF_NULL(shader);
    if (active != NULL) {
        GLint id;
        glGetIntegerv(GL_CURRENT_PROGRAM, &id);
        *active = shader->program == id;
    } 
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_GetUserPointer(RPGshader *shader, void **user) {
    RPG_RETURN_IF_NULL(shader);
    if (user != NULL) {
        *user = shader->user;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Shader_SetUserPointer(RPGshader *shader, void *user) {
    RPG_RETURN_IF_NULL(shader);
    shader->user = user;
    return RPG_NO_ERROR; 
}