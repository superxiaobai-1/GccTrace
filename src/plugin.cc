#include "plugin.h"

#include <options.h>
#include <tree-check.h>
#include <tree-pass.h>
#include <tree.h>

#include <cp/cp-tree.h>

#include "c-family/c-pragma.h"
#include "cpplib.h"

int plugin_is_GPL_compatible = 1;

namespace GccTrace {

void cb_finish_parse_function(void *gcc_data, void *user_data) {
  tree decl = (tree)gcc_data;
  auto expanded_location = expand_location(decl->decl_minimal.locus);
  auto decl_name = decl_as_string(decl, 0);
  auto parent_decl = DECL_CONTEXT(decl);
  const char *scope_name = nullptr;
  GccTrace::EventCategory scope_type = GccTrace::EventCategory::UNKNOWN;
  if (parent_decl) {
    if (TREE_CODE(parent_decl) != TRANSLATION_UNIT_DECL) {
      scope_name = decl_as_string(parent_decl, 0);
      switch (TREE_CODE(parent_decl)) {
        case NAMESPACE_DECL:
          scope_type = GccTrace::EventCategory::NAMESPACE;
          break;
        case RECORD_TYPE:
          scope_type = GccTrace::EventCategory::STRUCT;
          break;
        case UNION_TYPE:
          scope_type = GccTrace::EventCategory::STRUCT;
          break;
        default:
          fprintf(stderr, "Unkown tree code %d\n", TREE_CODE(parent_decl));
          break;
      }
    }
  }
  end_parse_function(FinishedFunction{
      gcc_data, decl_name, expanded_location.file, scope_name, scope_type});
}

void cb_plugin_finish(void *gcc_data, void *user_data) { write_all_events(); }

void (*old_file_change_cb)(cpp_reader *, const line_map_ordinary *);

void cb_file_change(cpp_reader *pfile, const line_map_ordinary *new_map) {
  if (new_map) {
    const char *file_name = ORDINARY_MAP_FILE_NAME(new_map);
    if (file_name) {
      switch (new_map->reason) {
        case LC_ENTER:
          start_preprocess_file(file_name, pfile);
          break;
        case LC_LEAVE:
          end_preprocess_file();
          break;
        default:
          break;
      }
    }
  }
  (*old_file_change_cb)(pfile, new_map);
}

void cb_start_compilation(void *gcc_data, void *user_data) {
  start_preprocess_file(main_input_filename, nullptr);
  cpp_callbacks *cpp_cbs = cpp_get_callbacks(parse_in);
  old_file_change_cb = cpp_cbs->file_change;
  cpp_cbs->file_change = &cb_file_change;
}

void cb_pass_execution(void *gcc_data, void *user_data) {
  auto pass = (opt_pass *)gcc_data;
  start_opt_pass(pass);
}

void cb_finish_decl(void *gcc_data, void *user_data) {
  finish_preprocessing_stage();
}

}  // namespace GccTrace

static const char *PLUGIN_NAME = "gperf";

bool setup_output(int argc, plugin_argument *argv) {
  const char *flag_name = "trace";
  const char *dir_flag_name = "trace-dir";
  // TODO: Maybe make the default filename related to the source filename.
  // TODO: Validate we only compile one TU at a time.
  FILE *trace_file = nullptr;
  if (argc == 0) {
    char file_template[] = "/tmp/trace_XXXXXX.json";
    int fd = mkstemps(file_template, 5);
    if (fd == -1) {
      perror("GPERF mkstemps error: ");
      return false;
    }
    trace_file = fdopen(fd, "w");
  } else if (argc == 1 && !strcmp(argv[0].key, flag_name)) {
    trace_file = fopen(argv[0].value, "w");
    if (!trace_file) {
      fprintf(stderr, "GPERF Error! Couldn't open %s for writing\n",
              argv[0].value);
    }
  } else if (argc == 1 && !strcmp(argv[0].key, dir_flag_name)) {
    std::string file_template{argv[0].value};
    file_template += "/trace_XXXXXX.json";
    int fd = mkstemps(file_template.data(), 5);
    if (fd == -1) {
      perror("GPERF mkstemps error: ");
      return false;
    }
    trace_file = fdopen(fd, "w");
  } else {
    fprintf(stderr,
            "GPERF Error! Arguments must be -fplugin-arg-%s-%s=FILENAME or "
            "-fplugin-arg-%s-%s=DIRECTORY\n",
            PLUGIN_NAME, flag_name, PLUGIN_NAME, dir_flag_name);
    return false;
  }
  if (trace_file) {
    GccTrace::init_output_file(trace_file);
    return true;
  } else {
    return false;
  }
}

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *ver) {
  static struct plugin_info gcc_trace_info = {
      .version = "V1.0", .help = "GccTrace time traces of the compilation."};
  GccTrace::COMPILATION_START = GccTrace::clock_t::now();
  if (!setup_output(plugin_info->argc, plugin_info->argv)) {
    return -1;
  }

  register_callback(PLUGIN_NAME, PLUGIN_INFO, nullptr, &gcc_trace_info);
  register_callback(PLUGIN_NAME, PLUGIN_START_UNIT,
                    &GccTrace::cb_start_compilation, nullptr);
  register_callback(PLUGIN_NAME, PLUGIN_FINISH_DECL, &GccTrace::cb_finish_decl,
                    nullptr);

  register_callback(PLUGIN_NAME, PLUGIN_FINISH_PARSE_FUNCTION,
                    &GccTrace::cb_finish_parse_function, nullptr);

  register_callback(PLUGIN_NAME, PLUGIN_PASS_EXECUTION,
                    &GccTrace::cb_pass_execution, nullptr);
  register_callback(PLUGIN_NAME, PLUGIN_FINISH, &GccTrace::cb_plugin_finish,
                    nullptr);

  return 0;
}