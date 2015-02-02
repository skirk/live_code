#include "glslprogram.h"
#include "gl_core_3_3.h"


void GLSLProgram::use()
{
    glUseProgram( handle );
}

int GLSLProgram::getHandle()
{
    return handle;
}

void GLSLProgram::bindAttribLocation( GLuint location, const char * name)
{
    glBindAttribLocation(handle, location, name);
}

void GLSLProgram::bindFragDataLocation( GLuint location, const char * name )
{
    glBindFragDataLocation(handle, location, name);
}

void GLSLProgram::setUniform( const char *name, float x, float y, float z)
{
    int loc = getUniformLocation(name);
    if( loc >= 0 ) {
        glUniform3f(loc,x,y,z);
    }
}

void GLSLProgram::setUniform( const char *name, const vec3 & v)
{
    this->setUniform(name,v.x,v.y,v.z);
}

void GLSLProgram::setUniform( const char *name, const vec4 & v)
{
    int loc = getUniformLocation(name);
    if( loc >= 0 ) {
        glUniform4f(loc,v.x,v.y,v.z,v.w);
    }
}

void GLSLProgram::setUniform( const char *name, const mat4 & m)
{
    int loc = getUniformLocation(name);
    if( loc >= 0 )
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
    }
}

void GLSLProgram::setUniform( const char *name, const mat3 & m)
{
    int loc = getUniformLocation(name);
    if( loc >= 0 )
    {
        glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
    }
}

void GLSLProgram::setUniform( const char *name, float val )
{
    int loc = getUniformLocation(name);
    if( loc >= 0 )
    {
        glUniform1f(loc, val);
    }
}

void GLSLProgram::setUniform( const char *name, int val )
{
    int loc = getUniformLocation(name);
    if( loc >= 0 )
    {
        glUniform1i(loc, val);
    }
}

void GLSLProgram::setUniform( const char *name, bool val )
{
    int loc = getUniformLocation(name);
    if( loc >= 0 )
    {
        glUniform1i(loc, val);
    }
}

void GLSLProgram::printActiveUniforms() { 
    GLint nUniforms, size, location, maxLen;
    GLchar * name;
    GLsizei written;
    GLenum type;

    glGetProgramiv( handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
    glGetProgramiv( handle, GL_ACTIVE_UNIFORMS, &nUniforms);

    name = (GLchar *) malloc( maxLen );

    printf(" Location | Name\n");
    printf("------------------------------------------------\n");
    for( int i = 0; i < nUniforms; ++i ) {
        glGetActiveUniform( handle, i, maxLen, &written, &size, &type, name );
        location = glGetUniformLocation(handle, name);
        printf(" %-8d | %s\n",location, name);
    }

    free(name);
}

void GLSLProgram::printActiveAttribs() {

    GLint written, size, location, maxLength, nAttribs;
    GLenum type;
    GLchar * name;

    glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
    glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &nAttribs);

    name = (GLchar *) malloc( maxLength );

    printf(" Index | Name\n");
    printf("------------------------------------------------\n");
    for( int i = 0; i < nAttribs; i++ ) {
        glGetActiveAttrib( handle, i, maxLength, &written, &size, &type, name );
        location = glGetAttribLocation(handle, name);
        printf(" %-5d | %s\n",location, name);
    }

    free(name);
}

int GLSLProgram::getUniformLocation(const char * name )
{
    return glGetUniformLocation(handle, name);
}



