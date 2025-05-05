#version 330

in	vec2	varyingPosition;
out vec4	outColor;
const float     M_PI = asin(1.0) * 2.0;

void main(void) {
	outColor = vec4(0.5 + varyingPosition.x, 0.5 + varyingPosition.y, 0.5, 1.0);
}