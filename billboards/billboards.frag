#version 150

uniform sampler2D tex0;

in vec2 UV;

out vec4 color;

void main(void)
{
  //color = vec4(0.5,0.1,0.1,1);
  color = texture(tex0, UV);
}
