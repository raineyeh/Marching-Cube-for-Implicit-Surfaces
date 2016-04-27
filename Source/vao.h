#ifndef __USEVAO_H__
#define __USEVAO_H__
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
void CreateBuffer(GLuint shader_program, GLuint& vao, GLuint& vbo, GLuint& ibo, GLuint& normal)
{	
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glGenBuffers(1, &normal);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);	

	GLint pos_loc = glGetAttribLocation(shader_program, "pos_attrib");
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, normal);
	GLint normal_loc = glGetAttribLocation(shader_program, "normal_attrib");
	glEnableVertexAttribArray(normal_loc);
	glVertexAttribPointer(normal_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
#endif