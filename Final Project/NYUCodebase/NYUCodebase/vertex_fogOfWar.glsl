attribute vec4 position;
attribute vec2 texCoord;
attribute float state;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

varying vec2 texCoordVar;
varying vec4 colorVar;

void main()
{
	colorVar = vec4(state, state, state, 1.0);
    texCoordVar = texCoord;
	gl_Position = projectionMatrix * modelviewMatrix  * position;
}