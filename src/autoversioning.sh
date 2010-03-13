#!/bin/bash

REVISION_FILE="version-revision.auto.inl"
NUMBER_FILE="version-number.auto.inl"
OLD_NUMBER_FILE="version-number.old.inl"
TIME_FILE="version-time.auto.inl"

OLD_REVISION="$(cat "${REVISION_FILE}")"
NEW_REVISION="\"$(svnversion)\""
echo "Revision: ${NEW_REVISION}"
echo "${NEW_REVISION}" > "${REVISION_FILE}"

if [[ "-${OLD_REVISION}" != "-${NEW_REVISION}" ]]
then
	(( NEW_NUMBER = 1 ))
else
	OLD_NUMBER="$(cat "${NUMBER_FILE}")"
	(( NEW_NUMBER = OLD_NUMBER + 1 ))
fi
echo "Build number: ${NEW_NUMBER}"
echo "${NEW_NUMBER}" > "${NUMBER_FILE}"

BUILD_TIME="$(date '+%s')"
echo "Build time: ${BUILD_TIME} ($(date -d "@${BUILD_TIME}"))"
echo "${BUILD_TIME}" > "${TIME_FILE}"
