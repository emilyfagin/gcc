/* efagin pass - identifies function clones
   Emily Fagin - Seneca Polytechnic College (2025)

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
#include <sstream>
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
#include "gimple-pretty-print.h"

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

// funcData holds the function's full name, as well as signature.
struct funcData { std::string full_name; std::string signature; };

/* 
   Currently, this pass generates a function signature based on 
   gimple statements, and other metrics. It detects function 
   clones using the base name of the function, and compares their 
   signature, outputting PRUNE or NOPRUNE based on analysis.
*/
class pass_efagin : public gimple_opt_pass
{
public:
  pass_efagin (gcc::context *ctxt): gimple_opt_pass 
    (pass_data_efagin, ctxt){}
  bool gate (function *) final override { return 1; }

  unsigned int execute (function *) final override;
	
private:  
  /* Stores function names to identify clones*/
  static std::unordered_map<std::string, std::vector<funcData>> 
    fun_map;

  std::string get_signature(function* fun);
};

// Initialize static member
std::unordered_map<std::string, std::vector<funcData>> 
  pass_efagin::fun_map;

// Generate a signature for a function
std::string pass_efagin::get_signature(function* fun) {
  std::stringstream signature;

  basic_block bb;
  int num_gimple_stmts = 0;

  FOR_EACH_BB_FN(bb, fun) {
    // iterate through all gimple statments in block
    for (gimple_stmt_iterator gsi = gsi_start_bb(bb); 
      !gsi_end_p(gsi); gsi_next(&gsi)) {
        
      // Record statement type with a space prefix
      signature << gimple_code(gsi_stmt(gsi)) << " ";
      num_gimple_stmts++;
    }
  }
    
  signature 
    << "\nBB\t" << n_basic_blocks_for_fn(fun) 
    << "\nEG\t" << n_edges_for_fn(fun) 
    << "\nGMP-S\t" << num_gimple_stmts;
  
  return signature.str();
}

unsigned int pass_efagin::execute(function* fun)
{
  // If dump file not enabled, exit
  if (!dump_file || !fun )
    return 0;

  // Get the full name of the function
  std::string name = IDENTIFIER_POINTER(DECL_NAME(fun->decl));
  fprintf(dump_file, "--------Examining Func: %s-----\n", 
    name.c_str());
	
  // Skip resolver functions
  if (name.find(".resolver") != std::string::npos) {
    fprintf(dump_file, "---End of Diagnostic (resolver)---\n");
    return 0;
  }

  // Extract base function name (remove everything after first dot)
  std::string base_name = name.find_first_of(".") 
    != std::string::npos 
    ? name.substr(0, name.find_first_of(".")) 
    : name;

  funcData fdata = {name, get_signature(fun)};
  fprintf(dump_file, "SIGNATURE:\n%s\n", fdata.signature.c_str());

  
  auto it = fun_map.find(base_name);
 
  // If this base name already has registered clones, print them
  if (it != fun_map.end()) {
    for (auto& variant : it->second) {
      fprintf(dump_file, "CLONE: %s\nCLONE SIGNATURE:\n%s\n", 
        variant.full_name.c_str(), variant.signature.c_str());
      
      if (fdata.signature == variant.signature) {
        fprintf(dump_file, "[%s] = PRUNE\n", base_name.c_str());
      } else {
        fprintf(dump_file, "[%s] = NOPRUNE\n", base_name.c_str());
      }
    }
  }
  
  // Add function data to the clones map
  fun_map[base_name].push_back(fdata);
  fprintf(dump_file, "--------End of Diagnostic---\n\n");

  return 0;
}

} // anonymous namespace

gimple_opt_pass *
make_pass_efagin (gcc::context *ctxt)
{
  return new pass_efagin (ctxt);
}
