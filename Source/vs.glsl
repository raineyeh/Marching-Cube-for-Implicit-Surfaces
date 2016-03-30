#version 450 core   
         
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
in vec3 pos_attrib;

void main(void)
{	
	gl_Position = P*V*M*vec4(pos_attrib,1.0);        
}