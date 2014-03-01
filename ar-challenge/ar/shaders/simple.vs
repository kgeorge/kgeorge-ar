#version 120

varying vec2 vTexCoord;
void main(void){
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vTexCoord = vec2(gl_MultiTexCoord0.xy);
}