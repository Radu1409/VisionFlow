#!/bin/bash
###############################################################################
#FILE:          check-code-style.sh
#DESCRIPTION:   Check and fix codestyle using uncrustify
#AUTHOR:        Radu Purecel <radu.purecel20@gmail.com>
###############################################################################

# envirinment prepartion:
CONFIG_NAME="uncrustify.cfg"
FORMATTER_DIR_NAME="formatter"
SUFFIX=".uncrustified"
LANGUAGE="C"
EXCLUDE="utest\|third-party"

CURRENT_DIR=$(pwd)
PROJECT_PATH="${CURRENT_DIR}/."
SOURCE_FILES=`find ${PROJECT_PATH} | grep '\.c$\|\.h$' | grep -v $EXCLUDE`
DIFFTOOL="meld -n"

CONFIG_PATH="${PROJECT_PATH}/${FORMATTER_DIR_NAME}/${CONFIG_NAME}"
SCRIPT_NAME=$0
DEPENDENCIES_LIST="meld uncrustify"
LOG_LEVEL="SILENT"
CLEANUP=false
AUTO_FIX=false
AUTO_CONFIRM=false

rc_eok="0"
rc_invalid_args="-1"
rc_missing_dependencies="-2"

rm_rc_missing_operand="123"

usage_examples ()
{
    echo -e "Useful examples:"
    echo -e ""
    echo -e "All *.c *.h files in path/to/sources/:"
    echo -e "./formatter/check-code-style.sh -f \"path/to/sources/*.[c|h]\""
    echo -e ""
    echo -e "Several files from path/to/sources/:"
    echo -e "./formatter/check-code-style.sh -f \"path/to/sources/header.h"
    echo -e "                                    path/to/sources/source.c\""
}

target_path_info ()
{
    echo -e "!!! Attention !!!"
    echo -e "It is strongly recommend to run this script specifying files to check via -f option."
    echo -e "Either you can provide a path to sources directory with -p option"
}

usage_info ()
{
    echo -e ""
    echo -e "This is code style validator/formatter wrapper over uncrustify"
    echo -e ""
    echo -e "Dependencies: ${DEPENDENCIES_LIST}"
    echo -e ""
    echo -e "Usage:   ./formatter/check-code-style.sh <options>"
    echo -e ""
    echo -e "Options:"
    echo -e "\t -c|--cleanup-only            clean generated files and exit"
    echo -e "\t -d|--difftool                (default 'meld')"
    echo -e "\t -f|--file <files>            (default all '*.c *.h')"
    echo -e "\t -p|--path <path to project>  (default current directory)"
    echo -e "\t -v|--verbose                 enable verbose mode"
    echo -e "\t -y|--yes                     auto confirm any other option"
    echo -e "\t -h|--help                    guess what it is..."
    echo -e ""
    usage_examples
    echo -e ""
    target_path_info
    echo -e ""
}


# logging
log_dbg ()
{
    if [[ ${LOG_LEVEL} == "DEBUG" ]]; then
        echo -e "DEBUG: $@"
    fi
}


# check dependencies:
ALL_DEPS_VALID=true
for APP in ${DEPENDENCIES_LIST}; do
    command -v ${APP} > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "\nApp: \"${APP}\" was not found"
        ALL_DEPS_VALID=false
    fi
done

if [ ${ALL_DEPS_VALID} = false ]; then
    echo -e "Not all needed dependencies found. Please install them"
    usage_info
    exit ${rc_missing_dependencies};
fi


# parse cmdline:
while [[ $# -gt 0 ]]
    do
    key="${1}"

    case $key in
        -a|-auto-fix|--auto-fix)
            AUTO_FIX=true
            shift 1 ;;

        -c|-cleanup-only|--cleanup-only)
            CLEANUP=true
            shift 1 ;;

        -d|--difftool)
            DIFFTOOL=${2}
            shift 2 ;;

        -f|-file|--file)
            SOURCE_FILES=${2}
            shift 2 ;;

        -p|-path|--path)
            PROJECT_PATH=${2}
            echo -e "New project path is: ${PROJECT_PATH}"
            SOURCE_FILES=`find ${PROJECT_PATH} | grep '\.c$\|\.h$'`
            shift 2 ;;

        -v|-verbose|--verbose)
            LOG_LEVEL="DEBUG"
            shift 1 ;;

        -y|-yes|--yes)
            AUTO_CONFIRM=true
            shift 1 ;;

        -h|-help|--help|-usage|--usage)
            usage_info
            exit -2;;

        *)
            echo -e "\nUnknown parameter. To get help run '${SCRIPT_NAME} -h'\nExiting...\n"
            exit -2
    esac
done


# check source files
if [[ -z ${SOURCE_FILES} ]]; then
    echo -e "====================================================================================="
    echo -e "Warning: you're running this script without specifying files for code style check"
    echo -e "So enjoy the absence of any guarantees of stability"
    echo -e "====================================================================================="
fi


# cleanup:
if [[ ${CLEANUP} == true ]]; then
    echo -e "Removing files with \"${SUFFIX}\" suffix..."
    find ${PROJECT_PATH} -name "*${SUFFIX}" | xargs rm -v 2> /dev/null #2>&1
    if [[ $? -eq ${rm_rc_missing_operand} ]]; then
        echo -e "\nNo files to remove!\n"
    fi
    echo -e "Done!"
    exit 0;
fi


# Check working directory. You can do here any check you want
if test -f ${CONFIG_PATH}; then
    echo    ""
    log_dbg "Found config file by this path: \"${CONFIG_PATH}\""
    log_dbg "Working directory is correct\n"
else
    echo -e ""
    echo -e "Can't find config file by this path: \"${CONFIG_PATH}\""
    echo -e "It seems that working directory is wrong, or config file is absent"
    exit -2;
fi


# do code style validation
uncrustify -l ${LANGUAGE} -c ${CONFIG_PATH} --suffix ${SUFFIX} ${SOURCE_FILES}
echo -e ""


# remove unchanged files, create list of files with issues
for FILE in ${SOURCE_FILES}; do
    UNCRUSTIFIED_FILE="${FILE}${SUFFIX}"

    log_dbg "Try to compare: \"${FILE}\""

    diff ${FILE} ${UNCRUSTIFIED_FILE} > /dev/null 2>&1 # suppress all output
    rc=$?
    if [ $rc -eq 0 ]; then
        log_dbg "File: \"${FILE}\" \tis unchanged. Removing..."
        rm ${UNCRUSTIFIED_FILE}

    elif [ $rc -eq 1 ]; then
        echo -e "File \"${FILE}\"\t has code style issue!"
        CHANGED_SOURCE_FILES="${CHANGED_SOURCE_FILES} ${FILE}"

    else
        echo -e "File was not found. Errcode: $rc. Exiting!"
        exit $?
    fi
done


# exit if no codestyle issue
if [[ -z ${CHANGED_SOURCE_FILES} ]]; then
    echo -e "\n<<< No code style issue found. You can commit your code >>>\n"
    exit 0;
fi

echo -e "\nCode style issues found."


# auto apply changes
if [[ ${AUTO_FIX} == true ]]; then
    if [[ ${AUTO_CONFIRM} == true ]]; then
        answer="yes"
    else
        echo -e "\nDo you really want to apply all changes that uncrustifier propose? (y/n)"
        read -r answer;
    fi
    if [[ "${answer} " == "y"* ]]; then
        echo -e "Ok, I hope you khow what you're doing, Applying changes..."
        for FILE in ${CHANGED_SOURCE_FILES}; do
            UNCRUSTIFIED_FILE="${FILE}${SUFFIX}"
            mv ${UNCRUSTIFIED_FILE} ${FILE}
        done
        echo "Done! Exiting..."
        exit 0;
    fi
fi

# open difftool
if [[ ${AUTO_CONFIRM} == true ]]; then
    answer="yes"
else
    echo -e "Do you want to view them via ${DIFFTOOL}? (y/N)"
    read -r answer;
fi
if [[ "${answer} " == "y"* ]]; then
    echo -e "Opening difftool..."
    for FILE in ${CHANGED_SOURCE_FILES}; do
        UNCRUSTIFIED_FILE="${FILE}${SUFFIX}"

        log_dbg "Open diff of: \"${FILE}\" and \"${UNCRUSTIFIED_FILE}\""
        if [[ ${DIFFTOOL} == "meld -n" ]]; then
            # if meld, script runs every diff in new tab and background mode
            meld -n ${FILE} ${UNCRUSTIFIED_FILE} & > /dev/null 2>&1
        else
            ${DIFFTOOL} ${FILE} ${UNCRUSTIFIED_FILE}
        fi
    done
else
    log_dbg "Skip diff vith \"${DIFFTOOL}\""
fi

echo "Done! Exiting..."
