project(
  'brawl',
  'c',
  version: '0.8',
  license: 'GPL',
  default_options: ['c_std=gnu99']
)

cc = meson.get_compiler('c')

math = cc.find_library('m')
gl = cc.find_library('GL')
glew = cc.find_library('GLEW')
glu = cc.find_library('GLU')
sdl = cc.find_library('SDL2')
sdlmixer = cc.find_library('SDL2_mixer')
freetype = cc.find_library('freetype')

brawl_dependencies = [math,gl,glew,sdl,sdlmixer,freetype]

brawl_version = '"@0@"'.format(meson.project_version())
pkg_datadir = join_paths(get_option('prefix'), get_option('datadir')) / 'brawl'
add_project_arguments('-DPKG_DATADIR="' + pkg_datadir + '"',
		      '-DBRAWL_VERSION=@0@'.format(brawl_version),
		      language: 'c')
add_project_arguments('-Wno-unused-but-set-variable', language: 'c')

if get_option('buildtype') == 'debug'
    add_project_arguments('-DDEBUG',language: 'c')
endif

brawl_inc = include_directories(
	'/usr/include',
	'/usr/include/GL',
	'/usr/include/SDL2',
	'/usr/include/freetype2',
	'src/brawl',
	'src/brawl/actor',
	'src/brawl/gl',
	'src/brawl/math',
	'src/ext',
	'src/zc_core',
	'src/zc_math',
	'src/zc_gfx',
	'src/ui')

com_sources = ['src/brawl/audio.c',
	       'src/brawl/bus.c',
	       'src/brawl/cmd.c',
	       'src/brawl/defaults.c',
	       'src/brawl/file.c',
	       'src/brawl/image.c',
	       'src/brawl/main.c',
	       'src/brawl/pivot.c',
	       'src/brawl/scene.c',
	       'src/brawl/scenerenderer.c',
	       'src/brawl/settings.c',
	       'src/brawl/str_util.c',
	       'src/brawl/view.c',

	       'src/brawl/actor/actor.c',
	       'src/brawl/actor/actor_ai.c',
	       'src/brawl/actor/actor_debug.c',
	       'src/brawl/actor/actor_group.c',
	       'src/brawl/actor/actor_hud_element.c',
	       'src/brawl/actor/actor_modifier_jump.c',
	       'src/brawl/actor/actor_modifier_jump_points.c',
	       'src/brawl/actor/actor_modifier_kick.c',
	       'src/brawl/actor/actor_modifier_ragdoll.c',
	       'src/brawl/actor/actor_modifier_types.c',
	       'src/brawl/actor/actor_modifier_walk.c',
	       'src/brawl/actor/actor_modifier_walk_points.c',
	       'src/brawl/actor/actor_skin.c',
	       'src/brawl/actor/base_metrics.c',
	       'src/brawl/actor/blood.c',
	       'src/brawl/actor/gun.c',

	       'src/brawl/gl/floatbuffer.c',
	       'src/brawl/gl/ogl.c',

	       'src/brawl/math/tween.c',
	       'src/brawl/math/physics2.c',
	       'src/brawl/math/triangulate.c',

	       'src/ui/cellrenderer.c',
	       'src/ui/element.c',
	       'src/ui/hudbar.c',
	       'src/ui/list.c',
	       'src/ui/listelement.c',
	       'src/ui/menuelement.c',
	       'src/ui/notifierelement.c',
	       'src/ui/sliderelement.c',
	       'src/ui/textelement.c',
	       'src/ui/ui.c',
	       'src/ui/uirenderer.c',
	       'src/ui/vectoranimator.c',

	       'src/zc_core/zc_bm_rgba.c',
	       'src/zc_core/zc_cstring.c',
	       'src/zc_core/zc_map.c',
	       'src/zc_core/zc_memory.c',
	       'src/zc_core/zc_string.c',
	       'src/zc_core/zc_vector.c',
	       'src/zc_core/zc_log.c',
	       'src/zc_core/zc_time.c',
	       'src/zc_core/zc_path.c',
	       'src/zc_core/zc_channel.c',
	       'src/zc_core/zc_bm_rgba.c',
	       'src/zc_core/zc_wrapper.c',

	       'src/zc_math/zc_vec2.c',
	       'src/zc_math/zc_util2.c',
	       'src/zc_math/zc_vec3.c',
	       'src/zc_math/zc_util3.c',
	       'src/zc_math/zc_vec4.c',
	       'src/zc_math/zc_mat4.c',

	       'src/zc_gfx/zc_draw.c',
	       'src/zc_gfx/zc_text.c',

	       'src/ext/nanosvg.h',
	       'src/ext/stb_image.h']
	       
install_data('res/Impact.ttf', install_dir : pkg_datadir )
install_data('res/argh0.wav', install_dir : pkg_datadir )
install_data('res/argh1.wav', install_dir : pkg_datadir )
install_data('res/argh2.wav', install_dir : pkg_datadir )
install_data('res/blockicon.png', install_dir : pkg_datadir )
install_data('res/death0.wav', install_dir : pkg_datadir )
install_data('res/death1.wav', install_dir : pkg_datadir )
install_data('res/downarrow.png', install_dir : pkg_datadir )
install_data('res/hiticon.png', install_dir : pkg_datadir )
install_data('res/kickicon.png', install_dir : pkg_datadir )
install_data('res/leftarrow.png', install_dir : pkg_datadir )
install_data('res/level0.svg', install_dir : pkg_datadir )
install_data('res/level1.svg', install_dir : pkg_datadir )
install_data('res/level2.svg', install_dir : pkg_datadir )
install_data('res/level3.svg', install_dir : pkg_datadir )
install_data('res/level4.svg', install_dir : pkg_datadir )
install_data('res/level5.svg', install_dir : pkg_datadir )
install_data('res/level6.svg', install_dir : pkg_datadir )
install_data('res/names.txt', install_dir : pkg_datadir )
install_data('res/punch0.wav', install_dir : pkg_datadir )
install_data('res/punch1.wav', install_dir : pkg_datadir )
install_data('res/punch2.wav', install_dir : pkg_datadir )
install_data('res/rightarrow.png', install_dir : pkg_datadir )
install_data('res/shooticon.png', install_dir : pkg_datadir )
install_data('res/shot.wav', install_dir : pkg_datadir )
install_data('res/theme.wav', install_dir : pkg_datadir )
install_data('res/uparrow.png', install_dir : pkg_datadir )

executable(
  'brawl',
  com_sources,
  include_directories: [brawl_inc],
  dependencies: brawl_dependencies,
  install: true,
)