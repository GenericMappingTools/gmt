#!/bin/bash
#
# Push HTML pages to the gh-pages branch of the current Github repository.
#
# Keeps pages built from branches in separate folders (named after the branch name).
# Pages for the master branch are in the 'dev' folder. 'latest' is a link to
# the 6.x branch.

# To return a failure if any commands inside fail
set -e
set -x

REPO=$BUILD_REPOSITORY_NAME
BRANCH=gh-pages
CLONE_DIR=deploy
CLONE_ARGS="--quiet --branch=$BRANCH --single-branch"
REPO_URL=https://${GITHUB_TOKEN}@github.com/${REPO}.git
HTML_SRC=${BUILD_SOURCESDIRECTORY}/${HTML_BUILDDIR:-doc/_build/html}

# Place the HTML in different folders for different branches
# Only deploy master and 6.x branches
if [[ "${BUILD_SOURCEBRANCHNAME}" =~ ^6\.[0-9]+$ ]]; then
    VERSION=${BUILD_SOURCEBRANCHNAME}
elif [[ "${BUILD_SOURCEBRANCHNAME}" == "master" ]]; then
    VERSION=dev
else
    echo -e "Not in master or 6.x branches. Deployment skipped."
    exit 0
fi

echo -e "DEPLOYING HTML TO GITHUB PAGES:"
echo -e "Target: branch ${BRANCH} of ${REPO}"
echo -e "HTML source: ${HTML_SRC}"
echo -e "HTML destination: ${VERSION}"

# Clone the project, using the secret token.
# Uses /dev/null to avoid leaking decrypted key.
echo -e "Cloning ${REPO}"
git clone ${CLONE_ARGS} ${REPO_URL} ${CLONE_DIR} 2>&1 >/dev/null

cd ${CLONE_DIR}

# Configure git to a dummy Travis user
git config user.email "AzurePipelines@nothing.com"
git config user.name "AzurePipelines"

# Delete all the files and replace with our new set
echo -e "Remove old files from previous builds"
rm -rf ${VERSION}
cp -Rf ${HTML_SRC}/ ${VERSION}/

# Need to have this file so that Github doesn't try to run Jekyll
touch .nojekyll

# Always link /latest to the 6.x branch docs
if [[ "${VERSION}" != "dev" ]]; then
    echo -e "Setup link from ${VERSION} to 'latest'"
    rm -f latest
    ln -sf ${VERSION} latest
fi

echo -e "Add and commit changes"
git add -A .
git status
# Reuse the last commit if possible
if [[ $(git log -1 --format='%s') == *"${VERSION}"* ]]; then
    echo -e "Amending last commit"
    git commit --amend --reset-author --no-edit
else
    # Make a new commit
    echo -e "Making a new commit"
    git commit --allow-empty -m "Deploy $VERSION from Azure Pipelines"
fi

echo -e "Pushing..."
git push -fq origin $BRANCH 2>&1 >/dev/null

echo -e "Finished uploading generated files."

# Turn off exit on failure.
set +x
set +e
