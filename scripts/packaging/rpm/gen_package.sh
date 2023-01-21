#!/bin/bash

#sudo dnf install -y rpmdevtools rpmlint
#rpmdev-setuptree

sudo apt install -y rpm

mkdir -p ~/rpmbuild/
mkdir -p ~/rpmbuild/SOURCES/
mkdir -p ~/rpmbuild/SPECS/

# Get ENV variables
cd ../../
chmod +x env.sh
source env.sh
cd packaging/rpm


TMP_DIR=$LOUVRE_RPM_PACKAGE_NAME-$LOUVRE_VERSION
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

tar --create --file ~/rpmbuild/SOURCES/$TMP_DIR.tar.gz $TMP_DIR
cp louvre.spec ~/rpmbuild/SPECS/
cat ./../file_list.txt >> ~/rpmbuild/SPECS/louvre.spec
cat ./../dir_list.txt >> ~/rpmbuild/SPECS/louvre.spec

rpmbuild -bb ~/rpmbuild/SPECS/louvre.spec

rm -r $TMP_DIR

mkdir -p ../../../../louvre_tmp
mkdir -p ../../../../louvre_tmp/packages
mv ~/rpmbuild/RPMS/x86_64/*.rpm ../../../../louvre_tmp/packages
rm -r ~/rpmbuild
