#!/bin/bash

GH_URL="git@github.com:jdtournier/terminal_graphics.git"

set -e

rm -rf docs
git clone -b gh-pages $GH_URL docs
doxygen

(
    cd docs
    git add --all
    git commit -m "update doxygen docs"
    git push $GH_URL gh-pages
)
