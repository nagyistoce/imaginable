#!/bin/bash

BUILD_FILE="version-build.auto.inl"
REVISION_FILE="version-svn.auto.inl"

date '+%s' > "${BUILD_FILE}"

echo "\"$(svnversion)\"" > "${REVISION_FILE}"
