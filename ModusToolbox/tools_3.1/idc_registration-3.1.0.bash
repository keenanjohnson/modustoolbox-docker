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
  
UNINSTALL_SCRIPT="${PRODUCT_HOME}/uninstallers/uninstall_3.1.0.bash"
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
            # Skip for 2.4 releases
            if [[ "3.1" != "2.4" ]]; then
                # Need change the uninstall.bash owner and permission for IDC
                # detection if the Launcher should ask for sudo rights
                chown root:root "$UNINSTALL_SCRIPT"
                chmod 744 "$UNINSTALL_SCRIPT"
            fi
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
cat <<EOF > "${IDC_REGISTRY_DIR}/F691E06D-AA3E-478B-BE65-942AA9F6A7E9.json"
{
    "guid": "F691E06D-AA3E-478B-BE65-942AA9F6A7E9",
    "featureId": "com.ifx.tb.tool.modustoolbox",
    "type": "tool",
    "title": "ModusToolbox™ Tools Package 3.1",
    "version": "3.1.0.12257",
    "versionNumeric": 3010000012257,
    "path": "${PRODUCT_HOME}/tools_3.1/dashboard/bin/dashboard",
    "exePath": "${PRODUCT_HOME}/tools_3.1/dashboard/bin/dashboard",
    "toolImage": "${PRODUCT_HOME}/tools_3.1/tool-icon-3.1.0.png",
    "uninstallPath": "${UNINSTALLPATH_VALUE}",
    "description": "ModusToolbox™ is a set of multi-platform development tools and a comprehensive suite of GitHub-hosted firmware libraries. Together, they enable an immersive development experience for customers creating converged MCU and Wireless systems",
    "tags": "IDE, MTB, ModusToolbox",
    "help": "${PRODUCT_HOME}/docs_3.1/doc_landing.html",
    "licenses": "${PRODUCT_HOME}",
    "release": "${PRODUCT_HOME}/docs_3.1/mt_release_notes.pdf",
    "additionalPaths": [ { "name": "Project Creator 2.10.0", "path": "${PRODUCT_HOME}/tools_3.1/project-creator/project-creator" }, { "name": "Eclipse IDE for ModusToolbox™ 3.1", "path": "${PRODUCT_HOME}/ide_3.1/eclipse/modustoolbox-eclipse" } ],
    "dependencies": [  ]
}
EOF