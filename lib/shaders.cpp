#include <cstdio>
#include <cstdlib>

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>

#include "macs.hpp"
#include "macs-internals.hpp"


using namespace macs;
using namespace macs::internals;


shader::shader(type t)
{
    id = glCreateShader((int)t);

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

    char *mem = (char *)malloc(length + 1);
    fread(mem, 1, length, fp);
    mem[length] = 0;

    glShaderSource(id, 1, (const char **)&mem, NULL);

    free(mem);


    dbgprintf("[sh%u] Loaded shader from file\n", id);
}


bool shader::compile(void)
{
    glCompileShader(id);


    int illen;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &illen);
    if (illen > 1)
    {
        char *msg = (char *)malloc(illen + 1);
        glGetShaderInfoLog(id, illen, NULL, msg);
        msg[illen] = 0; // inb4 implementation bug

        dbgprintf("[sh%u] Shader compile message: %s", id, msg);

        free(msg);
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
        char *msg = (char *)malloc(illen + 1);
        glGetShaderInfoLog(id, illen, NULL, msg);
        msg[illen] = 0; // inb4 implementation bug

        dbgprintf("[pr%u] Program link message: %s", id, msg);

        free(msg);
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

    dbgprintf("[pr%u] Active.\n", id);
}


prg_uniform program::uniform(const char *name)
{
    return prg_uniform(glGetUniformLocation(id, name));
}


prg_uniform::prg_uniform(unsigned uni_id):
    id(uni_id)
{
}


void prg_uniform::operator=(const texture *t)
{
    glUniform1i(id, t->unit);
}
