#ifndef SHADERLIB_H
#define SHADERLIB_H


/* Windows static library */
#   ifdef SHADERLIB_STATIC

#       define SLAPI
#       define SLAPIENTRY

/* Link with Win32 static  lib */
#       if SHADERLIB_LIB_PRAGMAS
#           pragma comment (lib, "shaderlib_static.lib")
#       endif

/* Windows shared library (DLL) */
#   else

#       define SLAPIENTRY __stdcall
#       if defined(SHADERLIB_EXPORTS)
#           define SLAPI __declspec(dllexport)
#       else
#           define SLAPI __declspec(dllimport)
#           pragma comment (lib, "shaderlib.lib")
#       endif
#   endif

#pragma warning( disable : 4290 )


#include <GL/glew.h>
#include <string>
using std::string;
#include <map>

#include <glm/glm.hpp>
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;

#include <stdexcept>

class ShaderLibException : public std::runtime_error {
  public:
    ShaderLibException( const string & msg ) :
      std::runtime_error(msg) { }
};

namespace GLSLShader {
  enum GLSLShaderType {
    VERTEX = GL_VERTEX_SHADER, 
    FRAGMENT = GL_FRAGMENT_SHADER, 
    GEOMETRY = GL_GEOMETRY_SHADER,
    TESS_CONTROL = GL_TESS_CONTROL_SHADER, 
    TESS_EVALUATION = GL_TESS_EVALUATION_SHADER, 
    COMPUTE = GL_COMPUTE_SHADER
  };
};

template class SLAPI std::map<string, int>;
class SLAPI ShaderLib
{
  private:
    int  program;
    bool linked;
	
	std::map<string, int> uniformLocations;

    GLint  getUniformLocation(const char * name );
    bool fileExists( const string & fileName );
    string getExtension( const char * fileName );

    // Make these private in order to make the object non-copyable
    ShaderLib( const ShaderLib & other ) { }
    ShaderLib & operator=( const ShaderLib &other ) { return *this; }

  public:
    ShaderLib();
    ~ShaderLib();

    void   compileShader( const char *fileName ) throw (ShaderLibException);
    void   compileShader( const char * fileName, GLSLShader::GLSLShaderType type ) throw (ShaderLibException);
    void   compileShader( const string & source, GLSLShader::GLSLShaderType type, 
        const char *fileName = NULL ) throw (ShaderLibException);

    void   link() throw (ShaderLibException);
    void   validate() throw(ShaderLibException);
    void   use() throw (ShaderLibException);

    int    getProgram();
    bool   isLinked();

    void   bindAttribLocation( GLuint location, const char * name);
    void   bindFragDataLocation( GLuint location, const char * name );

    void   setUniform( const char *name, float x, float y, float z);
    void   setUniform( const char *name, const vec2 & v);
    void   setUniform( const char *name, const vec3 & v);
    void   setUniform( const char *name, const vec4 & v);
    void   setUniform( const char *name, const mat4 & m);
    void   setUniform( const char *name, const mat3 & m);
    void   setUniform( const char *name, float val );
    void   setUniform( const char *name, int val );
    void   setUniform( const char *name, bool val );
    void   setUniform( const char *name, GLuint val );

    void   printActiveUniforms();
    void   printActiveUniformBlocks();
    void   printActiveAttribs();

    const char * getTypeString( GLenum type );
};

#endif // SHADERLIB_H
