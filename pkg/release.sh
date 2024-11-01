#!/bin/bash

# cd into this script dir
export PKG_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Git repo base dir
export PROJ_DIR=$PKG_DIR/../

# Get date
export WEEK_DAY=$(LANG=C date +%a | awk '{print toupper(substr($0, 1, 1)) substr($0, 2)}')
export MONTH=$(LANG=C date +%b | awk '{print toupper(substr($0, 1, 1)) substr($0, 2)}')
export MONTH_DAY=$(date +%d)
export YEAR=$(date +%Y)

# Get version
export VERSION=$(cat $PROJ_DIR/VERSION)
export MAJOR="${VERSION%%.*}"
MINOR="${VERSION#*.}"
export MINOR="${MINOR%%.*}"
export PATCH="${VERSION##*.}"
export BUILD=$(cat $PROJ_DIR/BUILD)

# Get changelog
CHANGES=$(awk '/--/{exit} {print}' $PROJ_DIR/CHANGES)
CHANGES=$(echo "$CHANGES" | grep '*') 
CHANGES=$(echo "$CHANGES" | sed 's/^[ \t]*//') # Remove leading spaces
CHANGES=$(echo "$CHANGES" | sed 's/^*/-/g') # Replace * with -
export CHANGES=$(echo "$CHANGES" | grep '^-') # Filter out lines not starting with -

# Print values
echo "Version:" $MAJOR.$MINOR.$PATCH-$BUILD
echo "Date:" $WEEK_DAY $MONTH $MONTH_DAY $YEAR
echo "Changes:"
echo "$CHANGES"
echo ""

# Update fedora spec
$PKG_DIR/fedora/update_spec.sh

# Cleanup
cd $PKG_DIR
unset PKG_DIR
