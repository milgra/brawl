"\
attribute vec4 position;\
attribute vec4 color;\
varying vec4 colorv;\
varying vec4 positionv;\
uniform mat4 projection;\
void main ( )\
{\
	gl_Position = projection * position;\
	gl_PointSize = 8.0;\
	colorv = color;\
	positionv = position;\
}\
"
