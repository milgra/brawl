"\
varying vec4 colorv;\
varying vec4 positionv;\
void main( )\
{\
	gl_FragColor = colorv;\
   	if ( colorv.w == 1.0 && colorv.x < 0.8 )\
   	{\
       	float ver = sin(positionv.y)+1.0;\
       	float hor = sin(positionv.x)+1.0;\
       	float dia = sin((positionv.x+positionv.y)/1.5)+1.0;\
       	ver = floor(ver * 2.0)/4.0;\
       	hor = floor(hor * 2.0)/4.0;\
       	dia = floor(dia * 2.0)/4.0;\
       	if ( colorv.x >= colorv.y && colorv.x >= colorv.z ) gl_FragColor.x -= ver*0.05;\
       	if ( colorv.y >= colorv.x && colorv.y >= colorv.z ) gl_FragColor.y -= hor*0.05;\
		if ( colorv.z >= colorv.x && colorv.z >= colorv.y ) gl_FragColor.z -= dia*0.2;\
   	}\
}\
"
