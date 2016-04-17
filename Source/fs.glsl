#version 450 core

out vec4 fragcolor;     
uniform vec4 uFrontColor;     
uniform vec4 uBackColor;   

void main(void)
{   		
	if( gl_FrontFacing ){	
		fragcolor = uFrontColor ;
	} 
	else {
		fragcolor = uBackColor ;
	}	
}




















