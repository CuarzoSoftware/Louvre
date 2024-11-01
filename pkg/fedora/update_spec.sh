#!/bin/bash

if [ -z "$PKG_DIR" ]; then
    echo "Error: Run pkg/release.sh instead."
    exit 1
fi

cd $PKG_DIR/fedora

# Copy template
rm -f latest.spec
cp template.spec latest.spec

# Replace template placeholders
sed -i "s/__MAJOR__/$MAJOR/g" latest.spec
sed -i "s/__MINOR__/$MINOR/g" latest.spec
sed -i "s/__PATCH__/$PATCH/g" latest.spec
sed -i "s/__BUILD__/$BUILD/g" latest.spec
sed -i "s/__WEEK_DAY__/$WEEK_DAY/g" latest.spec
sed -i "s/__MONTH__/$MONTH/g" latest.spec
sed -i "s/__MONTH_DAY__/$MONTH_DAY/g" latest.spec
sed -i "s/__YEAR__/$YEAR/g" latest.spec
echo "$CHANGES" >> latest.spec

echo "Fedora: latest.spec updated."
