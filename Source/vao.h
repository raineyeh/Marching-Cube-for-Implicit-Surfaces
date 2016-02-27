#ifndef __USEVAO_H__
#define __USEVAO_H__

extern GLuint shader_program;
extern GLuint vao;
GLuint vbo,ibo;
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
void init_buffer()
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glGenVertexArrays(1, &vao);	

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo); 

	GLint pos_loc0 = glGetAttribLocation(shader_program, "pos_attrib"); 
	glEnableVertexAttribArray(pos_loc0); 
	glVertexAttribPointer(pos_loc0, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0)); 
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
#endif