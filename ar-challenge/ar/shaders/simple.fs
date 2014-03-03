#version 330
uniform sampler2D tex;
in vec2 vTexCoord;
out vec4 fragColor;
void main(void) {
    vec4 col = texture(tex, vTexCoord);
    fragColor = col;
}
