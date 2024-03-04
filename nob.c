#define NOB_IMPLEMENTATION
#include "src/nob.h"

static char *get_file_extension(const char *fileName) {
  char *dot = strrchr(fileName, '.');
  if (!dot || dot == fileName)
    return NULL;
  return dot;
}

const char *linux_compiler = "cc";
const char *windows_compiler = "x86_64-w64-mingw32-cc";

const char *linux_linker = "ar";
const char *windows_linker = "x86_64-w64-mingw32-ar";

typedef enum { PLATFORM_LINUX, PLATFORM_WINDOWS } build_platform_t;

bool build_raylib(build_platform_t platform) {
  static const char *raylib_modules[] = {
      "rcore",   "raudio", "rglfw",     "rmodels",
      "rshapes", "rtext",  "rtextures", "utils",
  };

  size_t temp_restore = nob_temp_save();

  const char *platform_string;
  if (platform == PLATFORM_LINUX)
    platform_string = "linux";
  else if (platform == PLATFORM_WINDOWS)
    platform_string = "windows";

  const char *compiler;
  if (platform == PLATFORM_LINUX)
    compiler = linux_compiler;
  else if (platform == PLATFORM_WINDOWS)
    compiler = windows_compiler;

  const char *build_dir = nob_temp_sprintf("build/raylib/%s", platform_string);

  if (!nob_mkdir_if_not_exists("build/raylib"))
    return false;

  if (!nob_mkdir_if_not_exists(build_dir))
    return false;

  bool result = true;

  Nob_Cmd cmd = {0};
  Nob_File_Paths object_files = {0};
  Nob_Procs procs = {0};

  for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i) {
    const char *input_path =
        nob_temp_sprintf("deps/raylib/src/%s.c", raylib_modules[i]);
    const char *output_path =
        nob_temp_sprintf("%s/%s.o", build_dir, raylib_modules[i]);

    nob_da_append(&object_files, output_path);

    if (nob_needs_rebuild(output_path, &input_path, 1)) {
      cmd.count = 0;
      nob_cmd_append(&cmd, compiler);
      nob_cmd_append(&cmd, "-ggdb", "-DPLATFORM_DESKTOP", "-fPIC");
      nob_cmd_append(&cmd, "-Ideps/raylib/src/external/glfw/include");
      nob_cmd_append(&cmd, "-c", input_path);
      nob_cmd_append(&cmd, "-o", output_path);
      Nob_Proc proc = nob_cmd_run_async(cmd);
      nob_da_append(&procs, proc);
    }
  }
  cmd.count = 0;

  if (!nob_procs_wait(procs))
    nob_return_defer(false);

  const char *libraylib_path = nob_temp_sprintf("%s/libraylib.a", build_dir);

  if (nob_needs_rebuild(libraylib_path, object_files.items,
                        object_files.count)) {
    const char *linker;
    if (platform == PLATFORM_LINUX)
      linker = linux_linker;
    else if (platform == PLATFORM_WINDOWS)
      linker = windows_linker;
    nob_cmd_append(&cmd, linker, "-crs", libraylib_path);
    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i) {
      const char *input_path =
          nob_temp_sprintf("%s/%s.o", build_dir, raylib_modules[i]);
      nob_cmd_append(&cmd, input_path);
    }
    if (!nob_cmd_run_sync(cmd))
      nob_return_defer(false);
  }

defer:
  nob_temp_rewind(temp_restore);
  nob_da_free(procs);
  nob_da_free(object_files);
  nob_cmd_free(cmd);
  return result;
}

bool build_lua(build_platform_t platform) {
  size_t temp_restore = nob_temp_save();

  const char *platform_string;
  if (platform == PLATFORM_LINUX)
    platform_string = "linux";
  else if (platform == PLATFORM_WINDOWS)
    platform_string = "windows";

  const char *compiler;
  if (platform == PLATFORM_LINUX)
    compiler = linux_compiler;
  else if (platform == PLATFORM_WINDOWS)
    compiler = windows_compiler;

  const char *build_dir = nob_temp_sprintf("build/lua/%s", platform_string);

  if (!nob_mkdir_if_not_exists("build/lua"))
    return false;

  if (!nob_mkdir_if_not_exists(build_dir))
    return false;

  bool result = true;

  Nob_Cmd cmd = {0};
  Nob_File_Paths source_files = {0};
  Nob_File_Paths object_files = {0};
  Nob_Procs procs = {0};

  {
    Nob_File_Paths temp = {0};
    nob_read_entire_dir("deps/lua/src/", &temp);
    for (size_t i = 0; i < temp.count; ++i)
      if (nob_get_file_type(nob_temp_sprintf(
              "deps/lua/src/%s", temp.items[i])) == NOB_FILE_REGULAR)
        if (strcmp(get_file_extension(temp.items[i]), ".c") == 0)
          nob_da_append(&source_files, temp.items[i]);
    nob_da_free(temp);
  }

  for (size_t i = 0; i < source_files.count; ++i) {
    const char *input_path =
        nob_temp_sprintf("deps/lua/src/%s", source_files.items[i]);
    get_file_extension(source_files.items[i])[1] = 'o';
    const char *output_path =
        nob_temp_sprintf("%s/%s", build_dir, source_files.items[i]);
    get_file_extension(source_files.items[i])[1] = 'c';

    nob_da_append(&object_files, output_path);

    if (nob_needs_rebuild(output_path, &input_path, 1)) {
      cmd.count = 0;
      nob_cmd_append(&cmd, compiler);
      nob_cmd_append(&cmd, "-ggdb", "-fPIC");
      nob_cmd_append(&cmd, "-Ideps/lua/src/");
      nob_cmd_append(&cmd, "-c", input_path);
      nob_cmd_append(&cmd, "-o", output_path);
      Nob_Proc proc = nob_cmd_run_async(cmd);
      nob_da_append(&procs, proc);
    }
  }
  cmd.count = 0;

  if (!nob_procs_wait(procs))
    nob_return_defer(false);

  const char *liblua_path = nob_temp_sprintf("%s/liblua.a", build_dir);

  if (nob_needs_rebuild(liblua_path, object_files.items, object_files.count)) {
    const char *linker;
    if (platform == PLATFORM_LINUX)
      linker = linux_linker;
    else if (platform == PLATFORM_WINDOWS)
      linker = windows_linker;
    nob_cmd_append(&cmd, linker, "-crs", liblua_path);
    for (size_t i = 0; i < object_files.count; ++i) {
      const char *input_path = object_files.items[i];
      nob_cmd_append(&cmd, input_path);
    }
    if (!nob_cmd_run_sync(cmd))
      nob_return_defer(false);
  }

defer:
  nob_temp_rewind(temp_restore);
  nob_da_free(procs);
  nob_da_free(object_files);
  nob_da_free(source_files);
  nob_cmd_free(cmd);
  return result;
}

const char *input_paths[] = {"src/main.c", "src/entity.c", "src/data.c"};

void usage(const char *program) {
  printf("%s [--windows | --linux] <-r> [args]", program);
  printf("\t--windows: Tries to compile for windows with mingw");
  printf("\t--linux: Tries to compile for linux with gcc");
  printf("\t-r: Tries to run the executable immediately after "
         "building, it passes everything that comes after it to the "
         "executable as arguments");
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  const char *program = nob_shift_args(&argc, &argv);
  (void)program;
#ifdef _WIN32
  build_platform_t platform = PLATFORM_WINDOWS;
#else
  build_platform_t platform = PLATFORM_LINUX;
#endif // _WIN32
  bool run_flag = false;

  while (argc > 0) {
    const char *subcmd = nob_shift_args(&argc, &argv);
    if (strcmp(subcmd, "--linux") == 0)
      platform = PLATFORM_LINUX;
    else if (strcmp(subcmd, "--windows") == 0)
      platform = PLATFORM_WINDOWS;
    else if (strcmp(subcmd, "-r") == 0) {
      run_flag = true;
      break;
    } else
      nob_log(NOB_ERROR, "Unknown flag %s", subcmd);
  }

  if (!nob_mkdir_if_not_exists("build"))
    return false;

  if (!build_raylib(platform))
    return 1;

  if (!build_lua(platform))
    return 1;

  const char *compiler;
  if (platform == PLATFORM_LINUX)
    compiler = linux_compiler;
  else if (platform == PLATFORM_WINDOWS)
    compiler = windows_compiler;

  const char *platform_string;
  if (platform == PLATFORM_LINUX)
    platform_string = "linux";
  else if (platform == PLATFORM_WINDOWS)
    platform_string = "windows";

  Nob_Cmd cmd = {0};
  if (nob_needs_rebuild("build/wooby", input_paths,
                        NOB_ARRAY_LEN(input_paths))) {
    nob_cmd_append(&cmd, compiler);
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    nob_cmd_append(&cmd, "-Ideps/raylib/src/");
    nob_cmd_append(&cmd, "-Ideps/lua/src/");
    nob_cmd_append(&cmd, "-o", "build/wooby");
    nob_da_append_many(&cmd, input_paths, NOB_ARRAY_LEN(input_paths));
    nob_cmd_append(&cmd,
                   nob_temp_sprintf("-Lbuild/raylib/%s", platform_string));
    nob_cmd_append(&cmd, nob_temp_sprintf("-Lbuild/lua/%s", platform_string));
    nob_cmd_append(&cmd, "-lraylib", "-lm");
    nob_cmd_append(&cmd, "-llua");
    if (platform == PLATFORM_WINDOWS) {
      nob_cmd_append(&cmd, "-lwinmm", "-lgdi32");
      nob_cmd_append(&cmd, "-static");
    }
    if (!nob_cmd_run_sync(cmd))
      return 1;
  } else {
    nob_log(NOB_INFO, "Executable is already up to date");
  }

  if (run_flag) {
    cmd.count = 0;
    nob_cmd_append(&cmd, "build/wooby");
    nob_da_append_many(&cmd, argv, argc);
    if (!nob_cmd_run_sync(cmd))
      return 1;
  }

  return 0;
}
