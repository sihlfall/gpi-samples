#!/bin/bash

# Set the library path
export LD_LIBRARY_PATH="/home/cluster/local/lib:/home/cluster/local/lib64:$LD_LIBRARY_PATH"

# Path to the binary you want to run
BINARY="/home/jbecker/git/gpi-samples/03_one_sided_comm/bin/one_sided_comm"

# Call the binary with all arguments passed to the script
"$BINARY" "$@"
