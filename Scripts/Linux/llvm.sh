#!/bin/bash
cd "$(dirname "$0")"
REPO_ROOT="$(pwd)"

source ./lib/enviorment_variables.sh
source ./lib/functions.sh
source ./lib/dist_determine.sh


prevent_privelage_run
set -e


case $1 in
    ""|help|--help|-h)show_help;exit;;
    -d|--deps|deps)
        for function in ${print_os_group_id_functions[@]}; do
            $function
        done
        pause
    ;;
    *)printf "${STY_RED}Unknown subcommand \"$1\".${STY_RST}\n";show_help;exit 1;;
esac