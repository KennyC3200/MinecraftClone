#version 330 core
out vec4 frag_color;

in vec2 uv_coords;

uniform sampler2D atlas;

void main() {
    frag_color = texture(atlas, uv_coords);
}