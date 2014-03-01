#version 120
uniform sampler2D backgroundTex;
varying vec2 vTexCoord;
void main(void) {
    gl_FragColor = texture2D(backgroundTex, vTexCoord);
}
