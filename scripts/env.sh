#!/bin/bash

export LOUVRE_DEB_PACKAGE_NAME=liblouvre-dev
export LOUVRE_RPM_PACKAGE_NAME=liblouvre-devel
export LOUVRE_DEB_PACKAGE_ARCH=amd64
export LOUVRE_RPM_PACKAGE_ARCH=x86_64
export LOUVRE_VERSION=`cat ./../VERSION`
export LOUVRE_BUILD=`cat ./../BUILD`
export LOUVRE_CHANGES=`cat ./../CHANGES`

