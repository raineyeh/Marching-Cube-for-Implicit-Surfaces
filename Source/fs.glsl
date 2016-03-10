#version 400
out vec4 fragcolor;   
in vec3 fColor;   
uniform float time;   
uniform vec4 ucolor;      
uniform int uTranslucent;
vec3 l_pos = vec3(0.0, 0.0, 20.0); //world space light pos
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
	vec3 n = normalize(n_vs);                       //normalize 
	vec3 l = normalize(l_pos - p_vs.xyz);			//world space light vector
	vec3 v = normalize(cam_pos_vs.xyz - p_vs.xyz);  //world space view vector
	vec3 r = reflect(-l, n);                        //world space reflection vector    	
		
	float f0 = (1.0 - Eta)/(1.0+Eta);
	float f1 = pow(Eta,2);	
	vec3 h = normalize(l+v);
	float nh2 = pow(dot(n,h),2);
	float tan2 = (1.0 - nh2)/nh2;
	float cos4 = pow(nh2,2);

	float F = f1 + (1.0-f1)*pow(1.0-dot(h,v),5);
	float D = pow(e,-tan2/m2)/(4*m2*cos4);
	float G = min(1, min(2.0*dot(n,h)*dot(n,v)/dot(v,h), 2.0*dot(n,h)*dot(n,l)/dot(v,h)));
	vec4 lightColor =  (1 - uTranslucent) * (kd*max(dot(n, l), 0.0) + ks*F*D*G/(pi*dot(n,v)));

	if( gl_FrontFacing ){
		fragcolor = ucolor + lightColor;
	} 
	else {
		fragcolor = vec4(0.2,0.2,0.2,0.5) + lightColor;
	}	
}




















