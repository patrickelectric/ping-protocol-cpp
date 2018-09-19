#!/bin/bash

# Deploy protocol for:
# - ping-arduino
# - ping-python

# Variables
bold=$(tput bold)
normal=$(tput sgr0)
script_path="$( cd "$(dirname "$0")" ; pwd -P )"
project_path=${script_path}/..
clone_folder=/tmp/update-repos

protocol_githash=$(git -C ${project_path} rev-parse HEAD)

repositories=(
    "ping-python"
)

# Functions
echob() {
    echo "${bold}${1}${normal}"
}

echob "Check build type."
# Do not build pull requests
if [[ ${TRAVIS_PULL_REQUEST} != "false" ]]; then
    echo "- Do not deploy PRs."
    exit 0
fi

echob "Check branch."
# Do only build master branch
if [[ ${TRAVIS_BRANCH} != "master" ]]; then
    echo "- Only master branch will be deployed."
    exit 0
fi

echob "Check git configuration."
if ! git config --list | grep -q "user.name"; then
    # Config for auto-building
    echo "- Git configuration does not exist, a new one will be configured."
    git remote rename origin upstream
    git config --global user.email "support@bluerobotics.com"
    git config --global user.name "BlueRobotics-CI"
    git config --global credential.helper "store --file=${HOME}/.git-credentials"
    echo "https://${GH_TOKEN}:@github.com" > ${HOME}/.git-credentials
else
    echo "- Git configuration already exist."
fi

echob "Build protocol."
if ! python ${project_path}/src/protocol/generate-python.py; then
    echo "- Protocol generation failed."
    exit 1
fi

echob "Clone repositories."
for repo in "${repositories[@]}"; do
    echo "- Clone ${repo}"
    rm -rf ${clone_folder}/${repo}
    git clone https://github.com/bluerobotics/${repo} ${clone_folder}/${repo}
done

echob "Update python repository."
mv ${project_path}/src/protocol/python/* /tmp/update-repos/ping-python/Ping/

echob "Commit and update changes in remote."
for repo in "${repositories[@]}"; do
    repo_path=/tmp/update-repos/${repo}
    echo "- Check ${repo}"
    echo $(git -C ${repo_path} status -s)
    if [[ $(git -C ${repo_path} status) ]]; then
        echo "- Something is different, a commit will be done."
        git -C ${repo_path} add --all
        COMMIT_MESSAGE="Update autogenerated files

From https://github.com/bluerobotics/ping-protocol/tree/"$protocol_githash
        git -C ${repo_path} commit -sm "${COMMIT_MESSAGE}"
    fi
    git -C ${repo_path} push origin master
done