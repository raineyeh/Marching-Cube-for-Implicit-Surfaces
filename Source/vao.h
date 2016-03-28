#ifndef __USEVAO_H__
#define __USEVAO_H__
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
void CreateBuffer(GLuint shader_program, GLuint& vao, GLuint& vbo, GLuint& ibo)
{	
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLint pos_loc = glGetAttribLocation(shader_program, "pos_attrib");
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//²»Ë¢Ë÷Òý
/*	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);
	glCreateBuffers(1, &ibo);
	GLint pos_loc = glGetAttribLocation(shader_program, "pos_attrib");
	glEnableVertexArrayAttrib(vao,pos_loc);
	glVertexArrayAttribBinding(vao, pos_loc, 0);
	glVertexArrayAttribFormat(vao, pos_loc, 3, GL_FLOAT, false, 0);
	glVertexArrayElementBuffer(vao, ibo);
	glVertexArrayVertexBuffer(vao, pos_loc, vbo, 0, sizeof(GL_FLOAT)* 3);
*/
}
#endif