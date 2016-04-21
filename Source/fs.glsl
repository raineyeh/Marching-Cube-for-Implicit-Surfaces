#version 450 core

out vec4 fragcolor;     
uniform vec4 uFrontColor;     
uniform vec4 uBackColor;   
uniform bool uTranslucent;
void main(void)
{   		
	if( gl_FrontFacing ){	
		fragcolor = uFrontColor ;
	} 
	else {
		fragcolor = uBackColor ;
	}	

	if(uTranslucent)
		fragcolor = uFrontColor ;
}




















