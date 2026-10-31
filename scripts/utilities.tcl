####################################################################################
#
# Distributed under MIT Licence
#   See https://github.com/house-of-abbey/Blackboard/blob/main/LICENCE.
#
####################################################################################
#
# TCL utilities for building projects.
#
# J D Abbey & P A Abbey, 15 May 2026
#
####################################################################################

# Get relative path and filename to target_file from source_path
# First argument is a file name, second a directory name (not checked)
#
# target_file: Full file path and name as used in Vivado
# source_path: Git directory from which we require the relative path
#
# Usage:
#  relTo "/a/b/c/d/z/file.ext" "/a/b/e/f"   => ../../c/d/z/file.ext
#  relTo "/a/b/c/d/z/file.ext" "/a/b/e"     => ../c/d/z/file.ext
#  relTo "/a/b/c/d/file.ext" "/a/b/e/f/g/h" => ../../../../c/d/file.ext
#
proc relTo {target_file source_path} {
  set tf [file split [file dirname $target_file]]
  set sp [file split $source_path]
  set i 1
  while {[lindex $tf $i] == [lindex $sp $i]} {
    incr i
  }
  set branch $i
  set np {}
  while {$i < [llength $sp]} {
    lappend np ".."
    incr i
  }
  lappend np {*}[lrange $tf $branch end]
  file join {*}$np [file tail $target_file]
}
