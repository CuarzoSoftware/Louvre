#!/bin/bash

# Get ENV variables
cd ../../
chmod +x env.sh
source env.sh
cd packaging/deb

TMP_DIR=$LOUVRE_DEB_PACKAGE_NAME-$LOUVRE_VERSION-$LOUVRE_BUILD

mkdir $TMP_DIR

# Copy files
while read f; do
  mkdir -p ./$TMP_DIR${f%/*}
  cp $f ./$TMP_DIR$f
done <./../file_list.txt

# Copy dirs
while read d; do
  mkdir -p ./$TMP_DIR${d%/*}
  cp -R $d ./$TMP_DIR${d%/*}
done <./../dir_list.txt

# Get size of files in kb
SIZE_KB=`du -s $TMP_DIR | cut -f1`

mkdir $TMP_DIR/DEBIAN
cp ./control $TMP_DIR/DEBIAN/control

# Update control file values
sed -i 's/LOUVRE_PACKAGE_NAME/'$LOUVRE_DEB_PACKAGE_NAME'/g' $TMP_DIR/DEBIAN/control
sed -i 's/LOUVRE_VERSION/'$LOUVRE_VERSION'/g' $TMP_DIR/DEBIAN/control
sed -i 's/LOUVRE_BUILD/'$LOUVRE_BUILD'/g' $TMP_DIR/DEBIAN/control
sed -i 's/LOUVRE_PACKAGE_SIZE/'$SIZE_KB'/g' $TMP_DIR/DEBIAN/control

# Creates .deb package
dpkg-deb --root-owner-group --build $TMP_DIR

# Clear tmp dir
rm -r $TMP_DIR

# Move .deb
mkdir -p ../../../../louvre_tmp
mkdir -p ../../../../louvre_tmp/packages
mv *.deb ../../../../louvre_tmp/packages

