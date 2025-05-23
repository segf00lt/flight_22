#define _UNITY_BUILD_

#define NOB_IMPLEMENTATION

#include "nob.h"
#include "basic.h"
#include "arena.h"
#include "context.h"
#include "str.h"
#include "array.h"
#include "os.h"


#define CC "clang"
#define DEV_FLAGS "-g", "-O0", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-D_UNITY_BUILD_", "-DDEBUG"
#define RELEASE_FLAGS "-O2", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define WASM_FLAGS "-Os", "-O2", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-Wno-pthreads-mem-growth", "-D_UNITY_BUILD_"
#define TARGET "bullet_hell.c"
#define EXE "bullet_hell"
#define LDFLAGS "-lraylib", "-lm", "-lpthread"

#if defined(OS_WINDOWS)
#error "windows support not implemented"
#elif defined(OS_MAC)
#define CTAGS "/opt/homebrew/bin/ctags"
#elif defined(OS_LINUX)
#define CTAGS "/usr/local/bin/ctags"
#else
#error "unsupported operating system"
#endif

#if defined(OS_WINDOWS)
#error "windows support not implemented"
#elif defined(OS_MAC)
#define EMCC "/opt/homebrew/bin/emcc"
#elif defined(OS_LINUX)
#define EMCC "emcc"
#else
#error "unsupported operating system"
#endif

#if defined(OS_WINDOWS)
#error "windows support not implemented"

#elif defined(OS_MAC)
#define STATIC_BUILD_LDFLAGS "-lm", "-framework", "IOKit", "-framework", "Cocoa", "-framework", "OpenGL"

#elif defined(OS_LINUX)

#define STATIC_BUILD_LDFLAGS "-lm"

#else
#error "unsupported operating system"
#endif

#define METAPROGRAM_EXE "metaprogram"

#if defined(OS_WINDOWS)
#error "windows support not implemented"

#elif defined(OS_MAC)

#define GAME_MODULE "bullet_hell.dylib"
#define SHARED "-dynamiclib"
#define SHARED_EXT ".dylib"

#elif defined(OS_LINUX)

#define GAME_MODULE "bullet_hell.so"
#define SHARED "-shared"
#define SHARED_EXT ".so"

#else
#error "unsupported operating system"
#endif

#define RAYLIB_DYNAMIC_LINK_OPTIONS "-L./third_party/raylib/build/",  "-I./third_party/raylib/", "-lraylib", "-Wl,-rpath,./third_party/raylib/build/"
#define RAYLIB_STATIC_LINK_OPTIONS "-I./third_party/raylib/build/", "-L./third_party/raylib/build/", "./third_party/raylib/build/libraylib.a"
#define RAYLIB_STATIC_LINK_WASM_OPTIONS "-I./third_party/raylib/build/", "-L./third_party/raylib/build/", "./third_party/raylib/build/libraylib.web.a"

#define RAYLIB_HEADERS "third_party/raylib/raylib.h", "third_party/raylib/raymath.h", "third_party/raylib/rlgl.h"


DECL_ARR_TYPE(Nob_Proc);

int bootstrap_project(void);
int build_metaprogram(void);
int run_metaprogram(void);
int build_hot_reload(void);
int build_static(void);
int build_release(void);
int build_wasm(void);
int build_itch(void);
int run_tags(void);
int build_raylib(void);
int build_raylib_shared(void);
int build_raylib_static(void);
int build_raylib_web(void);
int generate_vim_project_file(void);
int generate_nob_project_file(void);
int load_nob_project_file(void);
int build_raylib_new(void);

char _project_root_path[OS_PATH_LEN];
Str8 project_root_path;

int build_raylib_new(void) {
  nob_log(NOB_INFO, "building raylib");

  Nob_Cmd cmd = {0};

  ASSERT(os_set_current_dir_cstr("./third_party/raylib"));

  ASSERT(nob_mkdir_if_not_exists("./build"));

  Nob_File_Paths files_in_cur_dir = {0};

  ASSERT(nob_read_entire_dir(".", &files_in_cur_dir));

  {
    nob_log(NOB_INFO, "cleaning build artifacts");

    for(int i = 0; i < files_in_cur_dir.count; i++) scratch_scope() {
      Str8 f = scratch_push_str8_copy_cstr(files_in_cur_dir.items[i]);


      if(str8_ends_with(f, str8_lit(".o")) || str8_ends_with(f, str8_lit(".a")) || str8_ends_with(f, str8_lit(".dylib")) || str8_contains(f, str8_lit(".so"))) {
        ASSERT(os_remove_file(f));
      }

    }

  }

  /*
     clang -c rshapes.c -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -O1 -Werror=implicit-function-declaration  -I.  -Iexternal/glfw/include
     clang -c rtextures.c -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -O1 -Werror=implicit-function-declaration  -I.  -Iexternal/glfw/include
     clang -c rtext.c -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -O1 -Werror=implicit-function-declaration  -I.  -Iexternal/glfw/include
     clang -c utils.c -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -O1 -Werror=implicit-function-declaration  -I.  -Iexternal/glfw/include
     clang -x objective-c -c rglfw.c -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -O1 -Werror=implicit-function-declaration  -I.  -Iexternal/glfw/include -U_GNU_SOURCE
     clang -c rmodels.c -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -O1 -Werror=implicit-function-declaration  -I.  -Iexternal/glfw/include
     clang -c raudio.c -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -O1 -Werror=implicit-function-declaration  -I.  -Iexternal/glfw/include
     ar rcs ./libraylib.a rcore.o rshapes.o rtextures.o rtext.o utils.o rglfw.o rmodels.o raudio.o
     */

  char *raylib_files[] = {
    "rshapes",
    "rtextures",
    "rtext",
    "utils",
    "rglfw",
    "rmodels",
    "raudio",
  };

  char *raylib_static_cflags[] = {
    "-Wall",
    "-D_GNU_SOURCE",
    "-DPLATFORM_DESKTOP_GLFW",
    "-DGRAPHICS_API_OPENGL_33",
    "-Wno-missing-braces",
    "-Werror=pointer-arith",
    "-fno-strict-aliasing",
    "-std=c99",
    "-O1",
    "-Werror=implicit-function-declaration",
  };

  char *raylib_static_include_flags[] = {
    "-I.",
    "-Iexternal/glfw/include",
  };

  Nob_Proc compile_procs[ARRLEN(raylib_files)] = {0};

  for(int i = 0; i < ARRLEN(raylib_files); i++) scratch_scope() {

    Str8 f = scratch_push_str8f("%s.c", raylib_files[i]);
    char *f_cstr = (char*)(f.s);

    if(str8_match_lit("rglfw.c", f)) {
#if defined(OS_MAC)
      nob_cmd_append(&cmd, CC, "-x", "objective-c", "-c", f_cstr);
      nob_da_append_many(&cmd, raylib_static_cflags, ARRLEN(raylib_static_cflags));
      nob_da_append_many(&cmd, raylib_static_include_flags, ARRLEN(raylib_static_include_flags));
      nob_cmd_append(&cmd, "-U_GNU_SOURCE");
#elif defined(OS_LINUX)
      nob_cmd_append(&cmd, CC, "-c", f_cstr);
      nob_da_append_many(&cmd, raylib_static_cflags, ARRLEN(raylib_static_cflags));
      nob_da_append_many(&cmd, raylib_static_include_flags, ARRLEN(raylib_static_include_flags));
#elif defined(OS_WINDOWS)
#error windows not supported yet
#endif
    } else {
      nob_cmd_append(&cmd, CC, "-c", f_cstr);
      nob_da_append_many(&cmd, raylib_static_cflags, ARRLEN(raylib_static_cflags));
      nob_da_append_many(&cmd, raylib_static_include_flags, ARRLEN(raylib_static_include_flags));
    }

    compile_procs[i] = nob_cmd_run_async_and_reset(&cmd);

  }

  for(int i = 0; i < ARRLEN(compile_procs); i++) {
    ASSERT(nob_proc_wait(compile_procs[i]));
  }

  nob_cmd_append(&cmd, "ar", "rcs", "./libraylib.a");
  for(int i = 0; i < ARRLEN(raylib_files); i++) scratch_scope() {

    Str8 f = scratch_push_str8f("%s.o", raylib_files[i]);
    char *f_cstr = (char*)(f.s);

    nob_cmd_append(&cmd, f_cstr);

  }

  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(os_move_file(str8_lit("./libraylib.a"), str8_lit("./build/libraylib.a")));

  return 1;
}

int build_raylib(void) {
  nob_log(NOB_INFO, "building raylib");

  Nob_Cmd cmd = {0};

  ASSERT(nob_mkdir_if_not_exists("./third_party/raylib/build"));

  ASSERT(os_set_current_dir_cstr("./third_party/raylib"));

  // web
  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_WEB");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  os_move_file(str8_lit("./libraylib.web.a"), str8_lit("./build/libraylib.web.a"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  // static
  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(os_move_file(str8_lit("./libraylib.a"), str8_lit("./build/libraylib.a")));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  // shared
  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP", "RAYLIB_LIBTYPE=SHARED");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

#if defined(OS_WINDOWS)
#error windows not supported yet
#elif defined(OS_MAC)
  ASSERT(os_move_file(str8_lit("./libraylib.5.5.0.dylib"), str8_lit("./build/libraylib.5.5.0.dylib")));
  ASSERT(os_move_file(str8_lit("./libraylib.550.dylib"), str8_lit("./build/libraylib.550.dylib")));
  ASSERT(os_move_file(str8_lit("./libraylib.dylib"), str8_lit("./build/libraylib.dylib")));
#elif defined(OS_LINUX)
  ASSERT(os_move_file(str8_lit("./libraylib.so.5.5.0"), str8_lit("./build/libraylib.so.5.5.0")));
  ASSERT(os_move_file(str8_lit("./libraylib.so.550"), str8_lit("./build/libraylib.so.550")));
  ASSERT(os_move_file(str8_lit("./libraylib.so"), str8_lit("./build/libraylib.so")));
#endif

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(os_set_current_dir(project_root_path));

  return 1;
}

int build_raylib_static(void) {
  nob_log(NOB_INFO, "building raylib shared");

  Nob_Cmd cmd = {0};

  ASSERT(nob_mkdir_if_not_exists("./third_party/raylib/build"));

  ASSERT(os_set_current_dir_cstr("./third_party/raylib"));

  // static
  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  os_move_file(str8_lit("./libraylib.a"), str8_lit("./build/libraylib.a"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(os_set_current_dir(project_root_path));

  return 1;
}

int build_raylib_shared(void) {
  nob_log(NOB_INFO, "building raylib shared");

  Nob_Cmd cmd = {0};

  ASSERT(nob_mkdir_if_not_exists("./third_party/raylib/build"));

  ASSERT(os_set_current_dir_cstr("./third_party/raylib"));

  // shared
  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP", "RAYLIB_LIBTYPE=SHARED");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

#if defined(OS_WINDOWS)
#error windows not supported yet
#elif defined(OS_MAC)
  ASSERT(os_move_file(str8_lit("./libraylib.5.5.0.dylib"), str8_lit("./build/libraylib.5.5.0.dylib")));
  ASSERT(os_move_file(str8_lit("./libraylib.550.dylib"), str8_lit("./build/libraylib.550.dylib")));
  ASSERT(os_move_file(str8_lit("./libraylib.dylib"), str8_lit("./build/libraylib.dylib")));
#elif defined(OS_LINUX)
  ASSERT(os_move_file(str8_lit("./libraylib.so.5.5.0"), str8_lit("./build/libraylib.so.5.5.0")));
  ASSERT(os_move_file(str8_lit("./libraylib.so.550"), str8_lit("./build/libraylib.so.550")));
  ASSERT(os_move_file(str8_lit("./libraylib.so"), str8_lit("./build/libraylib.so")));
#endif

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(os_set_current_dir(project_root_path));

  return 1;
}

int build_raylib_web(void) {
  nob_log(NOB_INFO, "building raylib web");

  Nob_Cmd cmd = {0};

  ASSERT(nob_mkdir_if_not_exists("./third_party/raylib/build"));

  ASSERT(os_set_current_dir_cstr("./third_party/raylib"));

  // web
  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_WEB");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  os_move_file(str8_lit("./libraylib.web.a"), str8_lit("./build/libraylib.web.a"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(os_set_current_dir(project_root_path));

  return 1;
}

int build_metaprogram(void) {
  nob_log(NOB_INFO, "building metaprogram");

  run_tags();

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "metaprogram.c", "-o", METAPROGRAM_EXE, RAYLIB_STATIC_LINK_OPTIONS, STATIC_BUILD_LDFLAGS);

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int run_metaprogram(void) {
  nob_log(NOB_INFO, "running metaprogram");

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, scratch_push_cstrf("%S/metaprogram", project_root_path));

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int build_hot_reload(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in hot reload mode");

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", SHARED, "bullet_hell.c", RAYLIB_DYNAMIC_LINK_OPTIONS, "-o", GAME_MODULE, "-lm");
  Nob_Proc p1 = nob_cmd_run_async_and_reset(&cmd);

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "hot_reload_main.c", RAYLIB_DYNAMIC_LINK_OPTIONS, "-o", EXE, "-lm");

  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;
  if(!nob_proc_wait(p1)) return 0;

  return 1;
}

int build_static(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in static mode");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/static"));

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "static_main.c", RAYLIB_STATIC_LINK_OPTIONS, "-o", "./build/static/"EXE, STATIC_BUILD_LDFLAGS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_release(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in release mode");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/release"));

  nob_cmd_append(&cmd, CC, RELEASE_FLAGS, "static_main.c", RAYLIB_STATIC_LINK_OPTIONS, "-o", "./build/release/"EXE, STATIC_BUILD_LDFLAGS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_wasm(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building for WASM");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/wasm"));

  char *target = "wasm_main.c";
  nob_cmd_append(&cmd, EMCC, WASM_FLAGS, "--preload-file", "./aseprite/atlas.png", "--preload-file", "./sprites/islands.png", "--preload-file", "./sounds/", target, RAYLIB_STATIC_LINK_WASM_OPTIONS, RAYLIB_STATIC_LINK_WASM_OPTIONS, "-sEXPORTED_RUNTIME_METHODS=ccall", "-sUSE_GLFW=3", "-sFORCE_FILESYSTEM=1", "-sMODULARIZE=1", "-sWASM_WORKERS=1", "-sUSE_PTHREADS=1", "-sWASM=1", "-sEXPORT_ES6=1", "-sGL_ENABLE_GET_PROC_ADDRESS", "-sINVOKE_RUN=0", "-sNO_EXIT_RUNTIME=1", "-sMINIFY_HTML=0", "-o", "./build/wasm/bullet_hell.js", "-lpthread");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_itch(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building for itch.io");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/itch"));

  char *target = "wasm_main.c";
  nob_cmd_append(&cmd, EMCC, WASM_FLAGS, "--preload-file", "./aseprite/atlas.png", "--preload-file", "./sprites/the_sea.png", "--preload-file", "./sounds/", target, RAYLIB_STATIC_LINK_WASM_OPTIONS, RAYLIB_STATIC_LINK_WASM_OPTIONS, "-sEXPORTED_RUNTIME_METHODS=ccall,HEAPF32", "-sUSE_GLFW=3", "-sFORCE_FILESYSTEM=1", "-sMODULARIZE=1", "-sWASM_WORKERS=1", "-sUSE_PTHREADS=1", "-sWASM=1", "-sEXPORT_ES6=1", "--shell-file", "itch_shell.html", "-sGL_ENABLE_GET_PROC_ADDRESS", "-sINVOKE_RUN=1", "-sNO_EXIT_RUNTIME=1", "-sMINIFY_HTML=0", "-sASYNCIFY", "-o", "./build/itch/index.html", "-pthread", "-sALLOW_MEMORY_GROWTH",scratch_push_cstrf("-sSTACK_SIZE=%lu", MB(10)));
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "sh", "-c", "zip ./build/itch/flight_22.zip ./build/itch/*");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}
int run_tags(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, CTAGS, "-w", "--sort=yes", "--langmap=c:.c.h", "--languages=c", "--c-kinds=+zfxm", "--extras=-q", "--fields=+n", "--exclude=third_party", "-R");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, CTAGS, "-w", "--sort=yes", "--langmap=c:.c.h", "--languages=c", "--c-kinds=+zpxm", "--extras=-q", "--fields=+n", "-a", RAYLIB_HEADERS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}


#ifdef OS_WINDOWS

#error "windows support not implemented"

#elif defined(OS_MAC)

char vim_project_file[] =
  "let project_root = getcwd()\n"
  "let project_build = project_root . '/nob'\n"
  "let project_exe = '/bullet_hell'\n"
  "let project_run = project_root . project_exe\n"
  "let project_debug = 'open -a Visual\\ Studio\\ Code ' . project_root\n"
  "\n" 
  "let &makeprg = project_build\n"
  "\n"
  "nnoremap <F7> :call jobstart('open -a Terminal ' . project_root, { 'detach':v:true })<CR>\n"
  "nnoremap <F8> :call chdir(project_root)<CR>\n"
  "nnoremap <F9> :wa<CR>:make<CR>\n"
  "nnoremap <F10> :call StartScratchJob(project_run)<CR>\n"
  "nnoremap <F11> :call jobstart(project_debug, { 'detach':v:true })<CR>\n";

#elif defined(OS_LINUX)

char vim_project_file[] =
  "let project_root = getcwd()\n"
  "let project_build = project_root . '/nob'\n"
  "let project_exe = '/bullet_hell'\n"
  "let project_run = project_root . project_exe\n"
  "let project_debug = 'gf2 ' . project_root . project_exe\n"
  "\n"
  "let &makeprg = project_build\n"
  "\n"
  "nnoremap <F7> :call jobstart('alacritty --working-directory ' . project_root, { 'detach':v:true })<CR>\n"
  "nnoremap <F8> :call chdir(project_root)<CR>\n"
  "nnoremap <F9> :wa<CR>:make<CR>\n"
  "nnoremap <F10> :call StartScratchJob(project_run)<CR>\n"
  "nnoremap <F11> :call jobstart(project_debug, { 'detach':v:true })<CR>\n"
  "nnoremap <F12> :call jobstart('aseprite', { 'detach':v:true })<CR>\n";

#else
#error "unsupported operating system"
#endif

int generate_vim_project_file(void) {
  nob_log(NOB_INFO, "generating vim project file");
  Str8 path_str = scratch_push_str8f("%S/.project.vim", project_root_path);
  ASSERT(nob_write_entire_file((char*)path_str.s, vim_project_file, memory_strlen(vim_project_file)));
  return 1;
}

int generate_nob_vim_project_file(void) {
  nob_log(NOB_INFO, "generating nob project file");
  Str8 root_path = scratch_push_str8f("root = %S\n", os_get_current_dir());

  ASSERT(nob_write_entire_file(".project.nob", root_path.s, root_path.len));

  return 1;
}

int bootstrap_project(void) {
  nob_log(NOB_INFO, "bootstrapping project");

  generate_nob_vim_project_file();

  load_nob_project_file();

  generate_vim_project_file();

  //if(!build_raylib()) return 0;
  //if(!build_raylib_web()) return 0;
  if(!build_raylib_static()) return 0;
  if(!build_raylib_shared()) return 0;
  if(!build_metaprogram()) return 0;

  return 1;
}

// TODO make nob work in subdirs of the project dir
int load_nob_project_file(void) {
  Nob_String_Builder sb = {0};

  Str8 cur_dir = os_get_current_dir();

  for(;;) {
    if(!nob_read_entire_file(".project.nob", &sb)) {
      if(!nob_set_current_dir("..")) {
        nob_log(NOB_ERROR, "no .project.nob file found, please run bootstrap_project() before trying to build");
        return 1;
      }
    } else {
      os_set_current_dir(cur_dir);
      break;
    }
  }

  Nob_String_View sv = nob_sb_to_sv(sb);

  for(size_t i = 0; i < sv.count; i++) {
    if(sv.data[i] == '=') {
      Nob_String_View before = { .count = i, .data = sv.data };
      Nob_String_View after  = { .count = sv.count - (i+1), .data = sv.data + (i+1) };
      before = nob_sv_trim(before);
      after = nob_sv_trim(after);

      if(nob_sv_eq(before, nob_sv_lit("root"))) {
        project_root_path = (Str8){ .s = (u8*)after.data, .len = after.count };
      } else {
        nob_log(NOB_WARNING, "unknown option %s in .project.nob", before.data);
      }

    }
  }

  nob_log(NOB_INFO, "loaded .project.nob");

  return 1;
}

int main(int argc, char **argv) {

  context_init();

#if 0
  {

    NOB_GO_REBUILD_URSELF(argc, argv);

    bootstrap_project();

    return 0;
  }
#endif

  load_nob_project_file();

  ASSERT(os_set_current_dir(project_root_path));

  NOB_GO_REBUILD_URSELF(argc, argv);

  //if(!build_raylib()) return 1;
  if(!build_raylib_new()) return 1;
  //if(!build_raylib_shared()) return 1;
  //if(!build_raylib_static()) return 1;
  //if(!build_raylib_web()) return 1;
  //if(!build_metaprogram()) return 1;

  //run_metaprogram();
  //run_tags();

  //if(!build_release()) return 1;
  //if(!build_itch()) return 1;
  //if(!build_wasm()) return 1;
  //if(!build_hot_reload()) return 1;
  //if(!build_static()) return 1;


  return 0;
}
