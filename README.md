# Brawl - Mass Brawl!!!

![Brawl](brawl.png)

Brawl is a game about intense action with super realistic physics. You have to lead your gang to victory in a crime-infected town.

You have to improve your fighting skills continuously, learn to block attacks, punch or kick with the correct timing. Watch your health and energy state. You can pick up fainted opponents/friends and use guns.

FEATURES

- Intense action  
- Fluid animation  
- 6 different scenes  
- Responsive controls  
- Dozens of buddies and enemies  

The code is also suitable for learning purposes.

### Files

buffers.c - Vertex buffers for different layers of the scene  
bus.c - Message bus for inter-module communication  
defaults.c - Settings and defaults   
excavator.c - Route excavator for scene  
file.c - Config file handling  
floatbuffer.c - Float  collector for opengl vertex buffer  
generator.c - Scene generator  
levels.c - Level descriptions  
main.c - SDL handling, top level event handling  
maincube.c - Main cube state  
menu.c - Main menu handling  
ogl.c - OpenGL functions  
particle.c - Particle entity for effects
scene.c - Scene handler
ui.c - UI handler
voxelutil.c - Voxel generator for labels

### Tech info

Brawl is a pure C project written in [headerless C](https://github.com/milgra/headerlessc).  
It uses reference-counted memory management ( zc_memory.c ).  
It uses modular programming with mediator - implementation groups with different functionality that don't include each other directly, they send messages to each other.  
If a module is "static" a global variable is used to store it's state, instancing is meaningless .  

### TODO

- ui headerless  
- mtbmp to bm_rgba  
- re-write physics and collosion based on cljs-brawl  
- add snow and other new stuff from cljs-brawl  
- zc_util2 should show its a math lib 
- freetype for text rendering  
	2.5 brawl, first person nezet verekedes kozben lent jobbra  
	labfej tapadjon foldre  
	ne sebezzen a hulla amig viszed  
	rombolhato tereptargyak  
	intersection detection egyszerusitese  
