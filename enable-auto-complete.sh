#!/bin/bash

# Source this file to enable auto-complete for atbuild.sh

# Specify original script name
_script_name="atbuild.sh"

# Define available options for completion
_options=(
    "--build-target="
    "--build-type="
    "--enable-ut=all"
    "--enable-ut=partial"
    "--enable-valgrind"
    "--enable-test-coverage"
)

# Function for Bash completion
_atbuild_auto_completion() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    opts="${_options[*]}"

    COMPREPLY=( $(compgen -W "$opts" -- "$cur") )
    return 0
}

# Enable completion for the script
complete -F _atbuild_auto_completion $_script_name


# Or run these commands to enable auto-complete whenever a terminal starts
### echo "source /home/etrugia/workspace/itc-platform/atbuild-auto-complete.sh" >> ~/.bashrc
### source ~/.bashrc