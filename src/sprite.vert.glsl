#version 140

in vec4 A; //{posX,posY,texX,texY}

out vec2 TexCoord;

// projection matrix only needs to scale down coordinates to [-1..+1] range
#define PROJECTION_MATRIX mat4(mat2(1.0/240.0, 0.0,   0.0, 1.0/180.0))

void main() {
	gl_Position = PROJECTION_MATRIX * vec4(A.xy, 0.0, 1.0);
	TexCoord = A.zw;
}