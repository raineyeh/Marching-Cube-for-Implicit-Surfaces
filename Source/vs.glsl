#version 400            
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
	p_vs = M*vec4(pos_attrib, 1.0);  //world space vertex
	gl_Position = P*V*p_vs;          //clip space vertex position
   
	cam_pos_vs = inverse(V)*vec4(0.0, 0.0, 3.0, 1.0); ///world space camera/eye position (or could pass this as a uniform from the client application)
	n_vs = normalize(M*vec4(pos_attrib, 0.0)).xyz; //world space normal 
}