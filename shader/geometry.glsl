#version 330

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 6) out;

in vertexdata {
	vec2 TexCoord;
	vec2 Position;
} vertex[];

out vec3 Color;
out vec2 TexCoord;
out vec2 Position;

uniform float time;

mat4 rotationMatrix(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	return mat4(    oc * axis.x * axis.x + c,            //(0,0)
			oc * axis.x * axis.y - axis.z * s,   //(0,1)  
			oc * axis.z * axis.x + axis.y * s,   //(0,2) 
			0.0,				     //(0,3)
			oc * axis.x * axis.y + axis.z * s,   //(1,0)
			oc * axis.y * axis.y + c,	     //(1,1)
			oc * axis.y * axis.z - axis.x * s,   //(1,2)
			0.0,			             //(1,3)
			oc * axis.z * axis.x - axis.y * s,   //(2,0) 
			oc * axis.y * axis.z + axis.x * s,   //(2,1)
			oc * axis.z * axis.z + c,	     //(2,2)
			0.0,				     //(2,3)
			0.0, 0.0, 0.0, 1.0);		     //(3,*)
} 

mat4 translationMatrix(vec3 to) {
	return mat4(1.0, 0, 0, 0,
		    0, 1.0, 0, 0,
		    0, 0, 1.0, 0,
		    to.x, to.y, to.z, 1.0);
}

void main(void)
{
	int i;
	vec3 rotationaxis = vec3(0, 0, 1);

	vec4 center = gl_in[0].gl_Position + 
		(gl_in[5].gl_Position - gl_in[0].gl_Position)*0.5;
	vec4 pos = center;

	float v = 0.0;
	v += sin((center.x+time));
	v += sin((center.y+time)/2.0);
	v += sin((center.x+center.y+time)/2.0);
	//c += u_k/2.0 * vec2(sin(time/3.0), cos(time/2.0));
	v += sin(sqrt(center.x*center.x+center.y*center.y+1.0)+time);
	v = v/2.0;

	for (i=0; i<gl_in.length(); i++) {
		vec4 p1 = translationMatrix(vec3(center))  
			* rotationMatrix(rotationaxis, v )
			* translationMatrix(-vec3(center)) * gl_in[i].gl_Position;
		vec4 final = p1;
		gl_Position = final;
		Color = vec3(1, sin(3.14*v), cos(3.14*v));
		TexCoord = vertex[i].TexCoord;
		Position = vertex[i].Position;
		EmitVertex();
	}
	EndPrimitive();
}
