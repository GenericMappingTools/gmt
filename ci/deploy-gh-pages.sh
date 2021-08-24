#!/usr/bin/env bash
#
# Push HTML pages to the gh-pages branch of the current GitHub repository.
#
# Keeps pages built from branches in separate folders (named after the branch name).
# Pages for the master branch are in the 'dev' folder. 'latest' is a link to
# the 6.x branch.

# To return a failure if any commands inside fail
set -x -e

# Detect if this is a release or from the master branch
if [[ "$GITHUB_EVENT_NAME" == "release" ]]; then
    # Get the tag name without the "refs/tags/" part
    version="${GITHUB_REF#refs/*/}"
elif [[ "$GITHUB_EVENT_NAME" == "push" ]] && [[ "${GITHUB_REF#refs/*/}" =~ ^6\.[0-9]+$ ]]; then
    version="${GITHUB_REF#refs/*/}"
elif [[ "$GITHUB_EVENT_NAME" == "push" ]] && [[ "${GITHUB_REF#refs/*/}" == "master" ]]; then
    version=dev
else
    echo -e "Not in master or 6.x branches. Deployment skipped."
    exit 0
fi

echo "Deploying version: $version"
# Make the new commit message. Needs to happen before cd into deploy
# to get the right commit hash.
message="Deploy $version from $(git rev-parse --short HEAD)"
cd deploy
# Need to have this file so that Github doesn't try to run Jekyll
touch .nojekyll
# Delete all the files and replace with our new  set
echo -e "\nRemoving old files from previous builds of ${version}:"
rm -rvf ${version}
echo -e "\nCopying HTML files to ${version}:"
cp -Rvf ../build/doc/rst/html/ ${version}/
# If this is a new release, update the link from /latest to it
if [[ "${version}" != "dev" ]]; then
    echo -e "\nSetup link from ${version} to 'latest'."
    rm -f latest
    ln -sf ${version} latest
fi
# Stage the commit
git add -A .
echo -e "\nChanges to be applied:"
git status
# Configure git to be the GitHub Actions account
git config user.email "github-actions[bot]@users.noreply.github.com"
git config user.name "github-actions[bot]"
# If this is a dev build and the last commit was from a dev build
# (detect if "dev" was in the previous commit message), reuse the
# same commit
if [[ "${version}" == "dev" && `git log -1 --format='%s'` == *"dev"* ]]; then
    echo -e "\nAmending last commit:"
    git commit --amend --reset-author -m "$message"
else
    echo -e "\nMaking a new commit:"
    git commit -m "$message"
fi
# Make the push quiet just in case there is anything that could leak
# sensitive information.
echo -e "\nPushing changes to gh-pages."
git push -fq origin gh-pages 2>&1 >/dev/null
echo -e "\nFinished uploading generated files."

# Turn off exit on failure.
set +x +e
