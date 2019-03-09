#version 150

// attribute
in vec4 a_position;
in vec4 a_color;

// varying
out vec4 v_color;

void main()
{
  gl_Position = a_position;
  v_color = a_color;
}
