/* Test pass
   Language independent return value optimizations
   Copyright (C) 2004-2025 Free Software Foundation, Inc.


This file is part of GCC

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#define INCLUDE_MEMORY
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "ssa.h"
#include "tree-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "internal-fn.h"
#include "gimple-pretty-print.h"

// Added  headers:

#include "gimple.h"
#include "gimple-ssa.h"
#include "cgraph.h"
#include "attribs.h"
#include "pretty-print.h"
#include "tree-inline.h"
#include "intl.h"
#include "basic-block.h"


// for dump_printf
#include "tree-pretty-print.h"
#include "diagnostic.h"
#include "dumpfile.h"
#include "builtins.h"
#include <stdlib.h>


namespace {

const pass_data pass_data_efagin =
{
  GIMPLE_PASS, /* type */
  "efagin", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_NONE, /* tv_id */
  PROP_cfg, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0, /* todo_flags_finish */
};

class pass_efagin : public gimple_opt_pass
{
public:
  pass_efagin (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_efagin, ctxt)
  {}

  /* opt_pass methods: (--> all the time, so return 1)*/
  bool gate (function *) final override { return 1; }

  unsigned int execute (function *fun) final override {
    basic_block bb;
    struct cgraph_node *node;

    int func_cnt = 0, bb_cnt = 0, stmt_cnt = 0;

    // macro for iterating through all functions
    FOR_EACH_FUNCTION (node) {
      if (dump_file) { // if a dumpfile exists
        fprintf(dump_file, "=== Function %d Name '%s' ===\n", ++func_cnt, node->name());
      }
    }

    // macro for iterating through all basic blocks
    FOR_EACH_BB_FN(bb, fun) {
      bb_cnt++;
      if (dump_file) {
        fprintf(dump_file, "=== Basic block count: %d ===\n", bb_cnt);
      }
      for (gimple_stmt_iterator gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi)) {
        gimple *g = gsi_stmt (gsi);
        stmt_cnt++;
        if (dump_file) {
          fprintf (dump_file, "----- Statement Count: %d ------\n", stmt_cnt);
          print_gimple_stmt (dump_file, g, 0, TDF_VOPS|TDF_MEMSYMS);
        } 
      }
    }


    if (dump_file) {
      fprintf(dump_file, "\n\n## End efagin diagnost, strt regular dump of curr gimple###\n\n");
    }
    return 0;
  }

}; // class pass_efagin

} // anon namespace

gimple_opt_pass *
make_pass_efagin (gcc::context *ctxt)
{
  return new pass_efagin (ctxt);
}

