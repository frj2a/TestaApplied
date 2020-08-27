#! /bin/bash
if [ -e Makefile ]
then
    make distclean -s
    rm -fR GeneratedFiles release debug 2> /dev/null
fi
`which qmake` -r -spec linux-g++-64 CONFIG-="release debug" CONFIG+=release
echo -e - - -Compilando o testador de 'drives' '\033[1;33m'TestaApplied'\033[0m' para a Montadora... && \
make $MAKEOPTS -s && \
/usr/bin/strip release/TestaApplied
/usr/bin/upx --best --ultra-brute --all-methods --all-filters --preserve-build-id release/TestaApplied
mv release/TestaApplied ~/bin// && \
make distclean -s && \
rm -fR GeneratedFiles/ debug/ release/ 2> /dev/null
if [ -e /usr/bin/cloc ]
then
    /usr/bin/cloc *.cpp *.h
fi
