#version 140
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D costume;
layout (std140) uniform FX {
	float color;
	float brightness;
	float ghost;
	float pixelate;
	float mosaic;
	float fisheye;
	float whirl;
};

/* NOTE: Using these conversions probably isn't the fastest way to do
   the color and brightness effects */

/*GLSL Color Space Utility Functions
(c) 2015 tobspr
The MIT License (MIT)
Copyright (c) 2015 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#define saturate(v) clamp(v,0,1)

const float HCV_EPSILON=1e-10;
const float HSL_EPSILON=1e-10;

vec3 rgb_to_hcv(vec3 rgb) {
	// Based on work by Sam Hocevar and Emil Persson
	vec4 P = (rgb.g < rgb.b) ? vec4(rgb.bg, -1.0, 2.0/3.0) : vec4(rgb.gb, 0.0, -1.0/3.0);
	vec4 Q = (rgb.r < P.x) ? vec4(P.xyw, rgb.r) : vec4(rgb.r, P.yzx);
	float C = Q.x - min(Q.w, Q.y);
	float H = abs((Q.w - Q.y) / (6 * C + HCV_EPSILON) + Q.z);
	return vec3(H, C, Q.x);
}

vec3 hue_to_rgb(float hue) {
	float R = abs(hue * 6 - 3) - 1;
	float G = 2 - abs(hue * 6 - 2);
	float B = 2 - abs(hue * 6 - 4);
	return saturate(vec3(R,G,B));
}

vec4 hsl_to_rgb(vec4 c) {
	vec3 rgb = hue_to_rgb(c.x);
	float C = (1 - abs(2 * c.z - 1)) * c.y;
	return vec4((rgb - 0.5) * C + c.z, c.a);
}

vec4 rgb_to_hsl(vec4 c) {
	vec3 HCV = rgb_to_hcv(c.rgb);
	float L = HCV.z - HCV.y * 0.5;
	float S = HCV.y / (1 - abs(L * 2 - 1) + HSL_EPSILON);
	return vec4(HCV.x, S, L, c.a);
}

void main() {
	vec2 t = TexCoord*mosaic;
	/* perform pixelate effect here */
	vec4 c = rgb_to_hsl(texture(costume, t));

	if(c.a == 0) discard;
	c.x = mod(c.x + color, 1.0);
	c.z = clamp(c.z + brightness, 0.0, 1.0);
	c.a -= ghost;

	FragColor = hsl_to_rgb(c);
}