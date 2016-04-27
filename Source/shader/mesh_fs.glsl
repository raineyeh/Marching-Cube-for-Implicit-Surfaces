#version 400 
   
in vec3 GPosition;
in vec3 GNormal;
in vec3 n_vs;
in vec4 cam_pos_vs;
noperspective in vec3 GEdgeDistance;

out vec4 fragcolor;  

uniform vec4 uFrontColor;     
uniform vec4 uBackColor;   
uniform bool uTranslucent;
uniform vec4 uLineColor;

struct LightInfo {
  vec4 Position;  // Light position in eye coords.
  vec3 Intensity; // A,D,S intensity
};
uniform LightInfo Light;

struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
};
uniform MaterialInfo Material;

float pi = 3.1415926;
float e = 2.71828;
float Eta = 1.5;
float m2 = 1.0;
uniform vec4 ks = vec4(0.5, 0.5, 0.5, 1.0); // material specular color
uniform vec4 Ls = vec4(1.0, 1.0, 1.0, 1.0); // light source specular color
vec3 phongModel( vec3 pos, vec3 n )
{

	vec3 BaseColor;
	if( gl_FrontFacing ){	
		BaseColor = uFrontColor.rgb ;
	} 
	else {
		BaseColor = uBackColor.rgb  ;
	}
    vec3 l = normalize(Light.Position.xyz - pos);
    vec3 v = normalize(cam_pos_vs.xyz - pos);
    vec3 r = reflect( -l, n );
    vec3 ambient = Light.Intensity * BaseColor;
    float sDotN = max( dot(l,n), 0.0 );
    vec3 diffuse = Light.Intensity * BaseColor * sDotN;
	float f0 = (1.0 - Eta)/(1.0+Eta);
	float f1 = pow(Eta,2);	
	vec3 h = normalize(l+v);
	float nh2 = pow(dot(n,h),2);
	float tan2 = (1.0 - nh2)/nh2;
	float cos4 = pow(nh2,2);
	float F = f1 + (1.0-f1)*pow(1.0-dot(h,v),5);
	float D = pow(e,-tan2/m2)/(4*m2*cos4);
	float G = min(1, min(2.0*dot(n,h)*dot(n,v)/dot(v,h), 2.0*dot(n,h)*dot(n,l)/dot(v,h)));
	vec4 spec = ks*Ls*F*D*G/(pi*dot(n,v));
    return ambient + diffuse + spec.rgb;
}
void main(void)
{   		
	vec4 color = vec4( phongModel(GPosition, GNormal), 1.0 );
	float d = min( GEdgeDistance.x, GEdgeDistance.y );
    d = min( d, GEdgeDistance.z );
    float mixVal;	
    if( d < 0 ) {
        mixVal = 1.0;
		fragcolor = vec4(1,0,0,1);
    } else if( d > 2 ) {
        mixVal = 0.0;
		fragcolor = vec4(0,1,0,1);
    } else {        
        mixVal = exp2(-2.0 * (d*d));		
    }
	if(uTranslucent)
		color = uFrontColor ;
    fragcolor = mix(color, uLineColor,mixVal);			
}




















