/* efagin pass - identifies function clones
   Emily Fagin - Seneca Polytechnic College

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

#include <unordered_map>
#include <string>
#include <vector>
#include <stdlib.h>
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "ssa.h"
#include "gimple-iterator.h"
#include "internal-fn.h"
#include "cgraph.h"
#include "function.h"
#include "basic-block.h"

namespace {

// Pass metadata
const pass_data pass_data_efagin =
{
  GIMPLE_PASS,    /* type */
  "efagin",       /* name */
  OPTGROUP_NONE,  /* optinfo_flags */
  TV_NONE,        /* tv_id */
  PROP_cfg,       /* properties_required */
  0,              /* properties_provided */
  0,              /* properties_destroyed */
  0,              /* todo_flags_start */
  0,              /* todo_flags_finish */
};

/* Currently, this pass detects function clones during compilation.
   It identifies variants using the base name of the function, and outputs the analysis.
*/
class pass_efagin : public gimple_opt_pass
{
public:
  pass_efagin (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_efagin, ctxt)
  {}

  /* gate function: always execute */
  bool gate (function *) final override { return 1; }

  unsigned int execute (function *) final override;

private:
  /* Stores function names to identify clones*/
  static std::unordered_map<std::string, std::vector<std::string>> function_clones;

};

// Initialize static member
std::unordered_map<std::string, std::vector<std::string>> pass_efagin::function_clones;

unsigned int pass_efagin::execute(function* fun)
{
  // If dump file not enabled, exit
  if (!dump_file || !fun)
    return 0;

  // Get the full name of the function
  std::string name = IDENTIFIER_POINTER(DECL_NAME(fun->decl));
  fprintf(dump_file, "-------------Examining Func: %s--------\n", name.c_str());

  // Skip resolver functions
  if (name.find(".resolver") != std::string::npos) {
    fprintf(dump_file, "-------------End of Diagnostic (resolver function)-------------\n\n");
    return 0;
  }

  // Extract base function name (remove everything after first dot)
  std::string base_name = name.find_first_of(".") != std::string::npos
    ? name.substr(0, name.find_first_of("."))
    : name;

  auto it = function_clones.find(base_name);

  // If this base name already has registered clones, print them
  if (it != function_clones.end()) {
    for (const auto& variant : it->second)
      fprintf(dump_file, "CLONE IDENTIFIED: %s\n", variant.c_str());
    fprintf(dump_file, "CURRENT: %s\n\n", name.c_str());
  }

  // Add this function name to the list of variants for the base function
  function_clones[base_name].push_back(name);
  fprintf(dump_file, "-------------End of Diagnostic-------------\n\n");

  return 0;
}

} // anonymous namespace

gimple_opt_pass *
make_pass_efagin (gcc::context *ctxt)
{
  return new pass_efagin (ctxt);
}
