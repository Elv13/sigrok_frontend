#!/bin/sh

# Find icons in code

ICONS=`git grep -i  fromTheme | grep -oE 'fromTheme[ (A-Za-z]*"[^"]+"' |
    grep -oE '"[^"]+"' | cut -f2 -d'"' | sort | uniq`

echo '<RCC>' > mobile/icons/icons.qrc
echo '  <qresource prefix="icons">' >> mobile/icons/icons.qrc

for NAME in $ICONS; do
    FILE=$(find /usr/share/icons/breeze-dark -iname "${NAME}.*"| grep 22)
    if [ -e $FILE ]; then
        NEW_PATH=mobile/icons/$(echo $FILE | cut -f 6-99 -d'/')
        mkdir -p $(dirname $NEW_PATH)
        cp $FILE $NEW_PATH
        ICON_PATH=$(echo $NEW_PATH | cut -f3-99 -d '/')
        echo '    <file>'$ICON_PATH'</file>' >> mobile/icons/icons.qrc
    fi
done

echo '  </qresource>' >> mobile/icons/icons.qrc
echo '</RCC>' >> mobile/icons/icons.qrc
