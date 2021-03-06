#version 130

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform int xsize;
uniform int ysize;
uniform mat4 model_view;
uniform mat4 projection;

void main() 
{
	mat4 scale = mat4(1.0/33, 	0.0, 		0.0, 0.0,
			  0.0,  	1.0/33, 	0.0, 0.0,
			  0.0, 		0.0, 		1.0/33, 0.0,
			  0.0, 		0.0, 		0.0, 1.0 );
	mat4 back = mat4(xsize, 0.0, 0.0, 0.0,
			0.0, ysize, 0.0, 0.0,
			0.0, 0.0, xsize, 0.0,
			0.0, 0.0, 0.0, 1.0);
	
	// First, center the image by translating each vertex by half of the original window size
	// Then, multiply by the scale matrix to maintain size after the window is reshaped
	//vec4 newPos = vPosition + vec4(-70, -60, 0, 0);
	gl_Position = projection * model_view *scale* vPosition; 
	
	color = vColor;	
} 
