
#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../include/fat32_fsinfo.h"


FSInfo *create_fsinfo_sector() {
    FSInfo *fsinfo = (FSInfo *)malloc(sizeof(FSInfo));
    if (!fsinfo) {
        return NULL;
    }

    memset(fsinfo, 0, sizeof(FSInfo));

    fsinfo->FSI_LoadSig = 0x41615252;    // Lead signature
    fsinfo->FSI_StrucSig = 0x61417272;   // Structure signature
    fsinfo->FSI_Free_Count = 0xFFFFFFFF; // Unknown free cluster count
    fsinfo->FSI_Nxt_Free = 0xFFFFFFFF;   // Unknown next free cluster
    fsinfo->FSI_TrailSig = 0xAA550000;   // Trail signature

    return fsinfo;
}