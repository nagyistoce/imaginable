#!/bin/bash

BUILD_FILE="version-build.auto.inl"
REVISION_FILE="version-svn.auto.inl"

date '+%s' > "${BUILD_FILE}"

echo "\"$(svn log -r HEAD | head -n 2 | tail -n 1 | sed 's,^r\([^|]*\) | .*$,"\1",')\"" > "${REVISION_FILE}"
