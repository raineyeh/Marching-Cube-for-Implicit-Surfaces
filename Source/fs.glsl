#version 450 core
out vec4 fragcolor;   
in vec3 fColor;   
uniform float time;   
uniform vec4 ucolor;      
uniform int uTranslucent;
vec3 l_pos = vec3(10.0, 10.0, 20.0); //world space light pos
vec4 ks = vec4(0.5, 0.5, 0.5, 1.0); // material specular color
vec4 kd = vec4(0.9f, 0.5f, 0.3,1.0);

in vec3 n_vs;
in vec4 p_vs;
in vec4 cam_pos_vs;  
  
float pi = 3.1415926;
float e = 2.71828;
float Eta = 1.0;
float m2 = 0.2;

void main(void)
{   	
	if( gl_FrontFacing ){
		fragcolor = ucolor;
	} 
	else {
		fragcolor = vec4(0.2,0.2,0.2,0.5);
	}	
}




















