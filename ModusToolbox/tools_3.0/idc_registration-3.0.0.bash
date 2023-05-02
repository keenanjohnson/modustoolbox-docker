#!/bin/bash 
# This must be the first non-commented line in this script. It ensures
# bash doesn't choke on \r on Windows
(set -o igncr) 2>/dev/null && set -o igncr; # this comment is needed

# Allow unset variables
set +u

set +e

# Get path to the current script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Initially, set the path to the ModusToolbox location as the parent folder of the current script 
PRODUCT_HOME="$(cd "${SCRIPT_DIR}/.." && pwd)"

# if ModusToolbox location is set as script argument
if [[ $# -eq 1 ]]; then
    PRODUCT_HOME="$1"
fi
  
UNINSTALL_SCRIPT="${PRODUCT_HOME}/uninstallers/uninstall_3.0.0.bash"
UNINSTALLPATH_VALUE="'${UNINSTALL_SCRIPT}'"

# Define IDC Registry Location
case "$(uname -s)" in

    Darwin)

        IDC_REGISTRY_DIR="${HOME}/Library/Application Support/Infineon_Technologies_AG/Infineon-Toolbox"
        UNINSTALLPATH_VALUE="'${UNINSTALL_SCRIPT}' ${HOME}"

        # if the current script is run under the root rights 
        # then assume that it is "All users" installation
        if [ "$EUID" -eq 0 ]; then 
            IDC_REGISTRY_DIR="/Library/Application Support/Infineon_Technologies_AG/Infineon-Toolbox"
            UNINSTALLPATH_VALUE="'${UNINSTALL_SCRIPT}' /"
        fi

    ;;

    Linux)
        IDC_REGISTRY_DIR="${HOME}/.local/share/Infineon_Technologies_AG/Infineon-Toolbox"
        if [ "$EUID" -eq 0 ]; then 
            IDC_REGISTRY_DIR="/usr/local/share/Infineon_Technologies_AG/Infineon-Toolbox"
            # Need change the uninstall.bash owner and permission for IDC
            # detection if the Launcher should ask for sudo rights
            chown root:root "$UNINSTALL_SCRIPT"
            chmod 744 "$UNINSTALL_SCRIPT"
        fi

        UNINSTALLPATH_VALUE="'${UNINSTALL_SCRIPT}' zip"
    ;;

    CYGWIN*|MINGW32*|MSYS*)
        # Windows
        echo 'Windows not supported OS'
        exit 1
    ;;

    *)
        echo 'Unsupported OS'
        exit 1
    ;;
esac

# Create IDC Registry directory if not exist
mkdir -p "${IDC_REGISTRY_DIR}"

# Generate ModusToolbox <GUID>.json in IDC Register folder
cat <<EOF > "${IDC_REGISTRY_DIR}/6D96B99C-4704-496D-A673-11E4B03FE410.json"
{
    "guid": "6D96B99C-4704-496D-A673-11E4B03FE410",
    "featureId": "com.ifx.tb.tool.modustoolbox",
    "title": "ModusToolbox™ Tools Package 3.0.0",
    "version": "3.0.0.9369",
    "versionNumeric": 3000000009369,
    "path": "${PRODUCT_HOME}/ide_3.0/eclipse/modustoolbox-eclipse",
    "exePath": "${PRODUCT_HOME}/ide_3.0/eclipse/modustoolbox-eclipse",
    "toolImage": "${PRODUCT_HOME}/tools_3.0/tool-icon-3.0.0.png",
    "uninstallPath": "${UNINSTALLPATH_VALUE}",
    "description": "ModusToolbox™ is a set of multi-platform development tools and a comprehensive suite of GitHub-hosted firmware libraries. Together, they enable an immersive development experience for customers creating converged MCU and Wireless systems",
    "tags": "IDE, MTB, ModusToolbox",
    "help": "${PRODUCT_HOME}/docs_3.0/doc_landing.html",
    "licenses": "${PRODUCT_HOME}",
    "release": "${PRODUCT_HOME}/docs_3.0/mt_release_notes.pdf",
    "additionalPaths": [ { "name": "Project Creator 2.0.0", "path": "${PRODUCT_HOME}/tools_3.0/project-creator/project-creator" }, { "name": "Eclipse IDE for ModusToolbox™ 3.0", "path": "${PRODUCT_HOME}/ide_3.0/eclipse/modustoolbox-eclipse" } ]
}
EOF