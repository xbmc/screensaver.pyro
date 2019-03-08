#version 100

precision mediump float;

// attribute
attribute vec4 a_position;
attribute vec4 a_color;

// varying
varying vec4 v_color;

void main()
{
  gl_Position = a_position;
  v_color = a_color;
}
