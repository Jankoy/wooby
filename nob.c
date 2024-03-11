#define NOB_IMPLEMENTATION
#include "src/nob.h"

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
        nob_temp_sprintf("raylib/src/%s.c", raylib_modules[i]);
    const char *output_path =
        nob_temp_sprintf("%s/%s.o", build_dir, raylib_modules[i]);

    nob_da_append(&object_files, output_path);

    if (nob_needs_rebuild(output_path, &input_path, 1)) {
      cmd.count = 0;
      nob_cmd_append(&cmd, compiler);
      nob_cmd_append(&cmd, "-ggdb", "-DPLATFORM_DESKTOP", "-fPIC");
      nob_cmd_append(&cmd, "-Iraylib/src/external/glfw/include");
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

bool read_dir_recursively(const char *parent, Nob_File_Paths *children) {
  bool result = true;
  Nob_File_Paths temp = {0};

  if (nob_get_file_type(parent) != NOB_FILE_DIRECTORY)
    nob_return_defer(result);

  if (!nob_read_entire_dir(parent, &temp))
    nob_return_defer(false);

  for (size_t i = 0; i < temp.count; ++i) {
    const char *full_path = nob_temp_sprintf("%s/%s", parent, temp.items[i]);

    Nob_File_Type type = nob_get_file_type(full_path);
    if (type < 0)
      nob_return_defer(false);

    switch (type) {
    case NOB_FILE_DIRECTORY: {
      if (strcmp(temp.items[i], ".") == 0)
        continue;
      if (strcmp(temp.items[i], "..") == 0)
        continue;

      if (!read_dir_recursively(full_path, children))
        nob_return_defer(false);
    } break;

    case NOB_FILE_REGULAR: {
      nob_da_append(children, full_path);
    } break;

    case NOB_FILE_SYMLINK: {
    } break;

    case NOB_FILE_OTHER: {
      nob_log(NOB_ERROR, "Unsupported type of file %s", full_path);
      nob_return_defer(false);
    } break;

    default:
      NOB_ASSERT(0 && "unreachable");
    }
  }

defer:
  nob_da_free(temp);
  return result;
}

typedef struct {
  const char *file_path;
  size_t offset;
  size_t size;
} Resource;

typedef struct {
  Resource *items;
  size_t count;
  size_t capacity;
} Resources;

bool bundle_resources() {
  bool result = true;

  Nob_File_Paths resource_files = {0};

  if (!read_dir_recursively("resources", &resource_files))
    nob_return_defer(false);

  Resources resources = {0};
  Nob_String_Builder bundle = {0};
  Nob_String_Builder content = {0};
  FILE *out = NULL;

  for (size_t i = 0; i < resource_files.count; ++i) {
    content.count = 0;
    if (!nob_read_entire_file(resource_files.items[i], &content))
      nob_return_defer(false);
    nob_da_append(&resources, ((Resource){.file_path = resource_files.items[i],
                                          .offset = bundle.count,
                                          .size = content.count}));
    nob_da_append_many(&bundle, content.items, content.count);
    nob_da_append(&bundle, 0);
  }

  out = fopen("src/bundle.h", "w");

  fprintf(out, "#ifndef BUNDLE_H_\n");
  fprintf(out, "#define BUNDLE_H_\n");
  fprintf(out, "#include <stddef.h>\n");
  fprintf(out, "typedef struct {\n");
  fprintf(out, "  const char *file_path;\n");
  fprintf(out, "  size_t offset;\n");
  fprintf(out, "  size_t size;\n");
  fprintf(out, "} Resource;\n");
  fprintf(out, "const Resource resources[] = {\n");
  for (size_t i = 0; i < resources.count; ++i) {
    Resource res = resources.items[i];
    fprintf(
        out,
        "  (Resource){ .file_path = \"%s\", .offset = %zu, .size = %zu },\n",
        res.file_path, res.offset, res.size);
  }
  fprintf(out, "};\n");
  fprintf(out, "const size_t resources_count = %zu;\n", resources.count);
  fprintf(out, "const unsigned char bundle[] = {\n");
  const size_t row_size = 20;
  for (size_t row = 0; row < bundle.count / row_size; ++row) {
    fprintf(out, "  ");
    for (size_t col = 0; col < row_size; ++col) {
      size_t i = row * row_size + col;
      fprintf(out, "0x%02X, ", (unsigned char)bundle.items[i]);
    }
    fprintf(out, "\n");
  }
  size_t remainder = bundle.count % row_size;
  if (remainder > 0) {
    fprintf(out, "  ");
    for (size_t col = 0; col < remainder; ++col) {
      size_t i = bundle.count / row_size * row_size + col;
      fprintf(out, "0x%02X, ", (unsigned char)bundle.items[i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "};\n");
  fprintf(out, "#endif // BUNDLE_H_\n");

defer:
  if (out)
    fclose(out);

  free(content.items);
  free(bundle.items);
  free(resources.items);

  return result;
}

static void usage(const char *program) {
  printf("%s [--windows | --linux] <-r> [args]", program);
  printf("\t--windows: Tries to compile for windows with mingw");
  printf("\t--linux: Tries to compile for linux with gcc");
  printf("\t-r: Tries to run the executable immediately after "
         "building, it passes everything that comes after it to the "
         "executable as arguments");
}

static char *get_file_extension(const char *fileName) {
  char *dot = strrchr(fileName, '.');
  if (!dot || dot == fileName)
    return NULL;
  return dot;
}

static void strip_first_dir(Nob_File_Paths *paths) {
  for (size_t i = 0; i < paths->count; ++i)
    paths->items[i] = strchr(paths->items[i], '/') + 1;
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

  const char *build_path = nob_temp_sprintf("build/%s", platform_string);
  if (!nob_mkdir_if_not_exists(build_path))
    return 1;

  if (!nob_mkdir_if_not_exists(nob_temp_sprintf("%s/behaviors", build_path)))
    return 1;

  const char *exe;
  if (platform == PLATFORM_LINUX)
    exe = "build/wooby";
  else if (platform == PLATFORM_WINDOWS)
    exe = "build/wooby.exe";

  Nob_Cmd cmd = {0};
  Nob_File_Paths input_files = {0};
  Nob_File_Paths object_files = {0};
  Nob_Procs procs = {0};
  Nob_File_Paths inputs = {0};

  if (!read_dir_recursively("src", &inputs))
    return 1;

  strip_first_dir(&inputs);

  if (!bundle_resources())
    return 1;

  for (size_t i = 0; i < inputs.count; ++i) {
    if (strcmp(get_file_extension(inputs.items[i]), ".c") != 0)
      continue;

    const char *input_path = nob_temp_sprintf("src/%s", inputs.items[i]);
    nob_da_append(&input_files, nob_temp_strdup(input_path));
    char *temp_path = nob_temp_strdup(inputs.items[i]);
    get_file_extension(temp_path)[1] = 'o';
    const char *output_path =
        nob_temp_sprintf("build/%s/%s", platform_string, temp_path);
    nob_da_append(&object_files, output_path);

    if (nob_needs_rebuild(output_path, &input_path, 1)) {
      cmd.count = 0;
      nob_cmd_append(&cmd, compiler);
      nob_cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
      nob_cmd_append(&cmd, "-Iraylib/src/");
      nob_cmd_append(&cmd, "-c", input_path);
      nob_cmd_append(&cmd, "-o", output_path);
      Nob_Proc proc = nob_cmd_run_async(cmd);
      nob_da_append(&procs, proc);
    }
  }

  if (!nob_procs_wait(procs))
    return 1;

  cmd.count = 0;
  nob_cmd_append(&cmd, "rm", "src/bundle.h");
  if (!nob_cmd_run_sync(cmd))
    return 1;

  cmd.count = 0;
  if (nob_needs_rebuild(exe, object_files.items, object_files.count)) {
    nob_cmd_append(&cmd, compiler);
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    nob_cmd_append(&cmd, "-o", exe);
    nob_da_append_many(&cmd, object_files.items, object_files.count);
    nob_cmd_append(&cmd,
                   nob_temp_sprintf("-Lbuild/raylib/%s/", platform_string));
    nob_cmd_append(&cmd, "-lraylib", "-lm");
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
    nob_cmd_append(&cmd, exe);
    nob_da_append_many(&cmd, argv, argc);
    if (!nob_cmd_run_sync(cmd))
      return 1;
  }

  return 0;
}
