precision mediump float;
varying vec2 user_Tex;
varying vec4 user_Color;
uniform sampler2D ColorTex;
void main() {
  float alpha = texture2D(ColorTex, user_Tex).a;
  gl_FragColor = user_Color * alpha;
}