precision mediump float;
attribute vec2 vPos;
attribute vec2 vTex;
attribute vec4 vCol;
varying vec2 user_Tex;
varying vec4 user_Color;
uniform mat4 M;
void main() {
  gl_Position = M * vec4(vPos, 0.0, 1.0);
  user_Tex = vTex;
  user_Color = vCol;
}