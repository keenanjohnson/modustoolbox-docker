#!/bin/bash

# Current script direcotry
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Define ModusToolbox location as the parent to uninstallers folder
PRODUCT_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

SHORT_VERSION="3.1"
PATCH_VERSION="0" 
JSON_FILENAME="F691E06D-AA3E-478B-BE65-942AA9F6A7E9.json"

# if ModusToolbox location is set as script argument
# and location has ide_x.y folder (prevent from Infineon Toolbox uninstallPath argument pass)
if [[ $# -eq 1 ]] && [[ -d "${1}/tools_${SHORT_VERSION}" ]]; then
    PRODUCT_HOME="$1"
fi

# Detect the current user home directory
CURR_USER_HOME=$HOME
if [ "$USER" == "root" ] && [ -n "$SUDO_USER" ]; then
    CURR_USER_HOME=$(eval echo ~"$SUDO_USER")
fi

JSON_FILE="${CURR_USER_HOME}/.local/share/Infineon_Technologies_AG/Infineon-Toolbox/${JSON_FILENAME}"

# if the current script is run under the root rights 
# then try to find the JSON file in IDC Registry for All Users
if [ "$EUID" -eq 0 ]; then 
    IDC_REGISTRY_DIR="/usr/local/share/Infineon_Technologies_AG/Infineon-Toolbox"
    if [ -f "$IDC_REGISTRY_DIR/$JSON_FILENAME" ]; then
        JSON_FILE="$IDC_REGISTRY_DIR/$JSON_FILENAME"
    fi
fi

#LOG_FILE="/tmp/modustoolbox.uninstall.${NAME}.$(date +%Y%m%d-%H%M%S).log"
LOG_FILE=${LOG_FILE:-"/tmp/mtb.${SHORT_VERSION}.${PATCH_VERSION}.uninstall.$(date +%Y%m%d-%H%M%S).log"}

#######################################
# Echo current date and time
# Outputs:
#   Write current date, time and zone to stdout
#######################################
function get_current_datetime() { 
    date +'%Y-%m-%dT%H:%M:%S%z' 
}

#######################################
# Echo message to log file
# Globals:
#   LOG_FILE 
# Arguments:
#   Message to log
# Outputs:
#   Write message to log file 
function log_message() {
    echo "[$(get_current_datetime)]:" "$@" >> "${LOG_FILE}"
}

#######################################
# Remove files and folders listed in uninstall_x.y.dat 
# Arguments:
#   uninstall.dat file 
#######################################
function process_uninstall_dat() {
    while IFS= read -r line || [[ -n "${line}" ]]; do
        # Skip empty and commented lines
        if [[ -z "${line}" ]] || [[ "${line}" == \#* ]]; then
            continue
        fi

        if [[ -d "${PRODUCT_HOME}/${line}" ]]; then
            log_message "Removing directory: ${PRODUCT_HOME}/${line}"
            rm -rf "${PRODUCT_HOME:?}/${line}" || log_message "Unable to delete: ${PRODUCT_HOME}/${line}"
        elif [[ -f "${PRODUCT_HOME}/${line}" ]]; then
            log_message "Removing file: ${PRODUCT_HOME}/${line}"
            rm -f "${PRODUCT_HOME}/${line}" || log_message "Unable to delete: ${PRODUCT_HOME}/${line}"
        fi
    done < "$1"
}

#######################################
# Run Patch uninstallers
# Arguments:
#   the current script arguments
#######################################
function run_patch_uninstallers() {
    _curr_script_filename=$(basename -- "${BASH_SOURCE[0]}")
    find "${SCRIPT_DIR}" -name "uninstall_${SHORT_VERSION}.*.bash" | while read line; do
        # skip the current uninstall.bash
        uninst_filename=$(basename -- "$line")

        [[ "$uninst_filename" == "$_curr_script_filename" ]]  && continue
        log_message "Executing Patch uninstall: $line"
        "$line" "$@"
    done
}

function main () {
    log_message "Uninstalling ModusToolbox $SHORT_VERSION.$PATCH_VERSION at: ${PRODUCT_HOME}"

    # Remove MTB content listed in uninstall_x.y.dat
    [[ -f "${SCRIPT_DIR}/uninstall_${SHORT_VERSION}.${PATCH_VERSION}.dat" ]] && process_uninstall_dat "${SCRIPT_DIR}/uninstall_${SHORT_VERSION}.${PATCH_VERSION}.dat"

    # Remove completely MTB folders for Base uninstaller
    if [[ "$PATCH_VERSION" == "0" ]]; then
        # Uninstall Patch releases
        run_patch_uninstallers "$@"

        rm -rf "${PRODUCT_HOME}/ide_${SHORT_VERSION}" || log_message "Unable to delete: ${PRODUCT_HOME}/ide_${SHORT_VERSION}"
        rm -rf "${PRODUCT_HOME}/tools_${SHORT_VERSION}" || log_message "Unable to delete: ${PRODUCT_HOME}/tools_${SHORT_VERSION}"
        rm -rf "${PRODUCT_HOME}/docs_${SHORT_VERSION}" || log_message "Unable to delete: ${PRODUCT_HOME}/docs_${SHORT_VERSION}"
        rm -f "${PRODUCT_HOME}/"CYPRESS*${SHORT_VERSION}.txt || log_message "Unable to delete: ${PRODUCT_HOME}/CYPRESS*${SHORT_VERSION}.txt"
        rm -f "${PRODUCT_HOME}/"README*_${SHORT_VERSION}.${PATCH_VERSION} || log_message "Unable to delete: ${PRODUCT_HOME}/README*_${SHORT_VERSION}.${PATCH_VERSION}"
    fi

    log_message "Removing from IDC Registry: ${JSON_FILE}"
    rm -f "${JSON_FILE}" || log_message "Unable to delete from IDC Registry: ${JSON_FILE}"

    log_message "Removing ModusToolbox $SHORT_VERSION uninstall script: ${BASH_SOURCE[0]}"
    rm -f "${SCRIPT_DIR}/uninstall_${SHORT_VERSION}.${PATCH_VERSION}.dat"  
    rm -f "${BASH_SOURCE[0]}"

    if [ -z "$(ls -A "${PRODUCT_HOME}/uninstallers")" ]; then
        log_message "Removing empty ModusToolbox uninstallers folder: ${PRODUCT_HOME}/uninstallers"
        rm -r "${PRODUCT_HOME}/uninstallers"
    fi

    if [ -z "$(ls -A "${PRODUCT_HOME}")" ]; then
        log_message "Removing empty ModusToolbox folder: ${PRODUCT_HOME}"
        rm -r "${PRODUCT_HOME}"
    fi
}

main "$@"