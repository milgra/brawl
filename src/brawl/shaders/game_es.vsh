"\
attribute highp vec4 position;\
attribute highp vec4 color;\
varying highp vec4 colorv;\
varying highp vec4 positionv;\
uniform mat4 projection;\
void main ( )\
{\
	gl_Position = projection * position;\
	gl_PointSize = 8.0;\
	colorv = color;\
	positionv = position;\
}\
"
