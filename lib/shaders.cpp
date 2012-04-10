#include <cstddef>
#include <cstdio>

#include <GL/gl.h>
#include <GL/glext.h>

#include "macs.hpp"
#include "macs-internals.hpp"


using namespace macs;
using namespace macs::internals;

namespace macs
{
    namespace internals
    {
        shader *basic_vertex_shader;
        program *basic_pipeline;
    }
}


shader::shader(type t)
{
    id = glCreateShader(static_cast<int>(t));

    dbgprintf("[sh%u] Is %s shader.\n", id, (t == vertex) ? "vertex" : "fragment");
}


shader::~shader(void)
{
    glDeleteShader(id);

    dbgprintf("[sh%u] Deleted.\n", id);
}


void shader::load(const char *src)
{
    glShaderSource(id, 1, &src, NULL);

    dbgprintf("[sh%u] Loaded shader from source\n", id);
}


void shader::load(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    rewind(fp);

    char *mem = new char[length + 1];

    fread(mem, 1, length, fp);
    mem[length] = 0;

    glShaderSource(id, 1, const_cast<const char **>(&mem), NULL);

    delete mem;


    dbgprintf("[sh%u] Loaded shader from file\n", id);
}


bool shader::compile(void)
{
    glCompileShader(id);


    int illen;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &illen);
    if (illen > 1)
    {
        char *msg = new char[illen + 1];

        glGetShaderInfoLog(id, illen, NULL, msg);
        msg[illen] = 0; // inb4 implementation bug

        dbgprintf("[sh%u] Shader compile message: %s", id, msg);

        delete msg;
    }


    int status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE)
        dbgprintf("[sh%u] Compilation successful.\n", id);
    else
        dbgprintf("[sh%u] Compilation failed.\n", id);

    return status == GL_TRUE;
}


program::program(void)
{
    id = glCreateProgram();

    dbgprintf("[pr%u] Created.\n", id);
}


program::~program(void)
{
    glDeleteProgram(id);

    dbgprintf("[pr%u] Deleted.\n", id);
}


void program::attach(shader *sh)
{
    glAttachShader(id, sh->id);

    dbgprintf("[pr%u] Attached shader sh%u.\n", id, sh->id);
}


bool program::link(void)
{
    glLinkProgram(id);


    int illen;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &illen);
    if (illen > 1)
    {
        char *msg = new char[illen + 1];

        glGetShaderInfoLog(id, illen, NULL, msg);
        msg[illen] = 0; // inb4 implementation bug

        dbgprintf("[pr%u] Program link message: %s", id, msg);

        delete msg;
    }


    int status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (status == GL_TRUE)
        dbgprintf("[pr%u] Linking successful.\n", id);
    else
        dbgprintf("[pr%u] Linking failed.\n", id);


    return status == GL_TRUE;
}


void program::use(void)
{
    glUseProgram(id);

//  dbgprintf("[pr%u] Active.\n", id);
}


prg_uniform program::uniform(const char *name)
{
    return prg_uniform(glGetUniformLocation(id, name));
}


prg_uniform::prg_uniform(unsigned uni_id):
    id(uni_id)
{
}


void prg_uniform::operator=(const in *o) throw(exc::invalid_type, exc::texture_not_assigned)
{
    switch (o->i_type)
    {
        case in::t_texture:
        case in::t_texture_array:
            for (int i = 0; i < tex_units; i++)
            {
                if ((*tmu_mgr)[i] == o)
                {
                    glUniform1i(id, i);
                    return;
                }
            }

            throw exc::tex_na;

        case in::t_vec4:
            glUniform4fv(id, 1, static_cast<const types::rps::vec4 *>(o)->v.d);
            return;

        case in::t_vec3:
            glUniform3fv(id, 1, static_cast<const types::rps::vec3 *>(o)->v.d);
            return;

        case in::t_mat4:
            glUniformMatrix4fv(id, 1, false, static_cast<const types::rps::mat4 *>(o)->m.d);
            return;

        case in::t_mat3:
            glUniformMatrix3fv(id, 1, false, static_cast<const types::rps::mat3 *>(o)->m.d);
            return;
    }

    throw exc::inv_type;
}
