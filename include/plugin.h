
#pragma once
#include <gcc-plugin.h>
#include <pretty-print.h>
#include "comm.h"
#include "perf_output.h"

namespace GccTrace {

void end_parse_function(FinishedFunction);
void write_all_scopes();
void write_all_functions();

} // namespace GccTrace