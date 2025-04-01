#pragma once

#include "perf_output.h"
#include "plugin.h"

namespace GccTrace {
void start_preprocess_file(const char *file_name, cpp_reader *pfile);
void end_preprocess_file();
void finish_preprocessing_stage();
void write_preprocessing_events();

void start_opt_pass(const opt_pass *pass);
void write_opt_pass_events();
}