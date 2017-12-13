attribute vec4 position;
attribute vec2 texCoord;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 colorVector;

varying vec2 texCoordVar;
varying vec4 colorVar;

void main()
{
    texCoordVar = texCoord;
	colorVar = colorVector;
	gl_Position = projectionMatrix * modelviewMatrix  * position;
}
