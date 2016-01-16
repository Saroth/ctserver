#include <stdio.h>
#include <config.h>

static BIO_INITLIST_T s_bio_initlist[] = {
    { "File control",   NULL,   bio_fctl_init,  (BIO_CHECK)bio_fctl,
        (unsigned long)&((BIO_FCTL_T *)0)->desc,  },
    { "Input/Output",   NULL,   bio_io_init,    (BIO_CHECK)bio_io,
        (unsigned long)&((BIO_IO_T *)0)->desc,  },
    { "Semaphore",      NULL,   bio_sem_init,   (BIO_CHECK)bio_sem,
        (unsigned long)&((BIO_SEM_T *)0)->desc,  },
};

int bio_init(void)
{
    int i = sizeof(s_bio_initlist) / sizeof(s_bio_initlist[0]);
    for(; i > 0; ) {
        i--;
        dbg_out_I(DS_BIO, "Init BIO type: %s", s_bio_initlist[i].type);
        s_bio_initlist[i].init(s_bio_initlist[i].io);
        void * p = s_bio_initlist[i].check();
        if(p == NULL) {
            dbg_out_E(DS_BIO_ERR, "IO init failed: %s", s_bio_initlist[i].type);
            return -1;
        }
        dbg_out_I(DS_BIO, "\t Success, get BIO description: %s",
                *(char **)(p + s_bio_initlist[i].desc_ofs));
    }
    return 0;
}

int bio_debug_init(void)
{
    return 0;
}

