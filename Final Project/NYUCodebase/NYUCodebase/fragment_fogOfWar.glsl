uniform sampler2D diffuse;

varying vec2 texCoordVar;
varying vec4 colorVar;

void main() {
    gl_FragColor = texture2D(diffuse, texCoordVar) * colorVar;
}