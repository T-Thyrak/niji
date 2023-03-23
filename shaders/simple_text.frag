#version 330 core

uniform sampler2D image;

in vec4 out_color;
in vec2 out_uv;

void main() {
    // gl_FragColor = texture(image, out_uv).x * out_color;
    float d = texture(image, out_uv).r;
    float aaf = fwidth(d);
    float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);
    gl_FragColor = vec4(out_color.rgb, alpha);
}
