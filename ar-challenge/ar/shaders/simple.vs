#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;


uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec2 vTexCoord;

void main(void){
    gl_Position = projection * view * model  * vec4(position.xy, 0.0, 1.0);
    vTexCoord = uv;
}