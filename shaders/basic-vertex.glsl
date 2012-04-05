varying vec2 tex_coord;

void main(void)
{
    gl_Position = gl_Vertex;
    tex_coord = .5 * (gl_Vertex.xy + vec2(1., 1.));
}
