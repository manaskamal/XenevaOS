#!/bin/bash
cd "$(dirname "$0")"
REPO_ROOT="$(pwd)"

source ./lib/enviorment_variables.sh
source ./lib/functions.sh
source ./lib/dist_determine.sh


prevent_privelage_run
set -e


case $1 in
    ""|-h|--help|help)show_help;exit;;
    -d|--deps|deps)

        # call print_os function to see if the os is supported by the script
        for function in ${print_os_group_id_functions[@]}; do
            $function
        done
        pause
        # initialize sudo session
        sudo_session      
        trap sudo_stop EXIT INT TERM
        source ./lib/distro/${OS_GROUP_ID}.sh
    ;;
    *)printf "${STY_RED}Unknown subcommand \"$1\".${STY_RST}\n";show_help;exit 1;;
esac