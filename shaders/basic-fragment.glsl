uniform sampler2D tex;

varying vec2 tex_coord;

void main(void)
{
    gl_FragColor = texture2D(tex, tex_coord);
}
