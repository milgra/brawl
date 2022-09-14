project(
  'brawl',
  'c',
  version: '0.8',
  license: 'GPL',
  default_options: ['c_std=gnu99']
)

cc = meson.get_compiler('c')

math = cc.find_library('m',static:false)
gl = cc.find_library('GL',static:false)
glew = cc.find_library('GLEW',static:false)
glu = cc.find_library('GLU',static:false)
sdl = cc.find_library('SDL2',static:false)
sdlmixer = cc.find_library('SDL2_mixer',static:false)

brawl_dependencies = [math,gl,glew,sdl,sdlmixer]

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
	'src/brawl',
	'src/brawl/actor',
	'src/brawl/ui',
	'src/modules',
	'src/modules/core',
	'src/modules/voxel',
	'src/modules/ext',
	'src/modules/math',
	'src/modules/utils',
	'src/modules/tools')

com_sources = ['src/brawl/audio.c',
	       'src/brawl/bridge.c',
	       'src/brawl/defaults.c',
	       'src/brawl/main.c',
	       'src/brawl/pivot.c',
	       'src/brawl/scene.c',
	       'src/brawl/scenerenderer.c',
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

	       'src/brawl/ui/cellrenderer.c',
	       'src/brawl/ui/element.c',
	       'src/brawl/ui/hudbar.c',
	       'src/brawl/ui/list.c',
	       'src/brawl/ui/listelement.c',
	       'src/brawl/ui/menuelement.c',
	       'src/brawl/ui/notifierelement.c',
	       'src/brawl/ui/sliderelement.c',
	       'src/brawl/ui/textelement.c',
	       'src/brawl/ui/ui.c',
	       'src/brawl/ui/uirenderer.c',
	       'src/brawl/ui/vectoranimator.c',

	       'src/modules/core/zc_bm_rgba.c',
	       'src/modules/core/zc_cstring.c',
	       'src/modules/core/zc_map.c',
	       'src/modules/core/zc_memory.c',
	       'src/modules/core/zc_string.c',
	       'src/modules/core/zc_vector.c',
	       'src/modules/core/zc_log.c',
	       'src/modules/core/zc_time.c',
	       'src/modules/core/zc_path.c',
	       'src/modules/core/zc_channel.c',
	       'src/modules/core/mtbmp.c',

	       'src/modules/utils/str_util.c',

	       'src/modules/voxel/bus.c',
	       'src/modules/voxel/file.c',
	       'src/modules/voxel/floatbuffer.c',
	       'src/modules/voxel/ogl.c',
	       'src/modules/voxel/settings.c',

	       'src/modules/math/zc_vec2.c',
	       'src/modules/math/zc_util2.c',
	       'src/modules/math/zc_vec3.c',
	       'src/modules/math/zc_util3.c',
	       'src/modules/math/zc_vec4.c',
	       'src/modules/math/zc_mat4.c',

	       'src/modules/ext/nanosvg.h',
	       'src/modules/ext/stb_image.h',
	       'src/modules/ext/stb_truetype.h',

	       'src/modules/tools/math1.c',
	       'src/modules/tools/cmd.c',
	       'src/modules/tools/font.c',
	       'src/modules/tools/image.c',
	       'src/modules/tools/physics2.c',
	       'src/modules/tools/text.c',
	       'src/modules/tools/triangulate.c']
	       
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