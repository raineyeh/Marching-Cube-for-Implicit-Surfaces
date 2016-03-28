#version 450 core            
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float time;
in vec3 pos_attrib;
out vec3 fColor;
out vec3 n_vs;
out vec4 p_vs;
out vec4 cam_pos_vs;

void main(void)
{	
	gl_Position = P*V*M*vec4(pos_attrib,1.0);        
}