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

audio.c - audio handling
bus.c - Message bus for inter-module communication  
cmd.c - Command queue   
defaults.c - Settings and defaults  
file.c - Config file handling  
image.c - Image loading  
main.c - SDL handling, top level event handling  
pivot.c - Pivot points in the scene  
scene.c - Scene handling
scenerenderer.c - Scene renderer
settings.c - Settings
str_util.c - String utils
view.c - View handler

actor - actor handling
gl - OpenGL handling
math - math functions

### Tech info

Brawl is a pure C project written in [headerless C](https://github.com/milgra/headerlessc).  
It uses reference-counted memory management ( zc_memory.c ).  
It uses modular programming with mediator - implementation groups with different functionality that don't include each other directly, they send messages to each other.  
If a module is "static" a global variable is used to store it's state, instancing is meaningless .  

### TODO

- blood not visible
- re-write physics and collosion based on cljs-brawl  
- add snow and other new stuff from cljs-brawl  
- zc_util2 should show its a math lib 
- labfej tapadjon foldre  
- ne sebezzen a hulla amig viszed  
- rombolhato tereptargyak  
- intersection detection egyszerusitese
- after first win multiply enemy count