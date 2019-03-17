#include <stdlib.h>
#include <stdio.h>
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <string.h>

int get_callinfo(char *fname, size_t fnlen, unsigned long long *ofs) {
    char f_name[fnlen];
    unw_cursor_t cursor;
    unw_context_t context;
    unw_word_t off, ip, sp;
    unw_proc_info_t pip;
    char procname[256];

    char mainf[] = "main";
    char libcmain[] = "__libc_start_main";
    int ret;
    if(unw_getcontext(&context))
        return -1;
    if(unw_init_local(&cursor, &context))
        return -1;
    for(int i = 0; i < 3; i++) {
        if(unw_step(&cursor) < 0) return -1;
        if(unw_get_proc_info(&cursor, &pip)) break;
        ret = unw_get_proc_name(&cursor, procname, 256, &off);
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        // fprintf(stderr, "procname = (%s), off = 0x%lx\n", procname, (long) off - 5);

    }
    *ofs = (unsigned long long) off - 5;
    ret = unw_get_proc_name(&cursor, procname, 256, &off);
    // fprintf(stderr, "last procname = (%s), off = 0x%lx\n", procname, (long) off - 5);

    // if(ret && ret != -UNW_ENOMEM) {
    //     procname[0] = '?';
    //     procname[1] = 0;
    // }

    if(strcmp(procname, libcmain) == 0) {
        strcpy(fname, mainf);
        // break;
    } else {
        strcpy(fname, procname);
    }


    return ret;
}

