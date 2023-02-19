#!/usr/bin/sh
# Если собирать под линухом в NSIS 3.04-1 - ругаются четыре движка на вирустотал. На собранные под виндой в NSIS 3.04 - нет(самоподписанные).
# И медленно очень, под виндой заметно быстрее.

find ./TorrServers -mindepth 1 -maxdepth 1 -type d -printf "-V3 -DTS_VERSION=%f ./offline_Installer.nsi\n" | xargs -L1 makensis 
