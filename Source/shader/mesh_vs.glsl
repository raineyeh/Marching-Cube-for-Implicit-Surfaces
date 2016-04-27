#version 400 
         
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 NormalMatrix;

in vec3 pos_attrib;
in vec3 normal_attrib;

out vec3 VNormal;
out vec3 VPosition;
out vec4 cam_pos_vs;

void main(void)
{		
	mat4 MVP = P*V*M;	
	VNormal = normalize( NormalMatrix * normal_attrib);
    VPosition = vec3(V*M * vec4(pos_attrib,1.0));
	cam_pos_vs = inverse(V)*vec4(0.0, 0.0, 3.0, 1.0);
	gl_Position = MVP * vec4(pos_attrib,1.0); 	
}