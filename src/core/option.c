#include <config.h>
#include <stdlib.h>
#include <getopt.h>

OPTS_T g_opts;                          //!< 程序选项

static int disp_help(void)
{
    dbg_out_I(MIO,
"Usage: %s [OPTIONS]\n"
"\n"
"           --help      Display this help and exit.\n"
"           --version   Display version informations and exit.\n"
,
PROGRAM_NAME);
    exit(EXIT_SUCCESS);
    return 0;
}
static int disp_version(void)
{
    dbg_out_I(MIO, "%s v%d.%d.%d (%s %s)", PROGRAM_NAME,
            (PROGRAM_VERSION >> 24) & 0xFF,
            (PROGRAM_VERSION >> 16) & 0xFF,
            (PROGRAM_VERSION >> 8) & 0xFF,
            __DATE__, __TIME__);
    exit(EXIT_SUCCESS);
    return 0;
}

/** \brief       选项处理 */
int option_parse(int argc, char * argv[])
{
    int err = 0;
    for(;;) {
        int opt_index = 0;
        static const char * short_opts = "";
        static const struct option long_opts[] = {
            { "help",       no_argument,    0,  0,  },
            { "version",    no_argument,    0,  0,  },
            { 0,            0,              0,  0,  },
        };
        int c = getopt_long(argc, argv, short_opts, long_opts, &opt_index);
        if(c == EOF) {
            break;
        }
        switch(c) {
            case 0: {
                switch(opt_index) {
                    case 0: {
                        disp_help();
                        break;
                    }
                    case 1: {
                        disp_version();
                        break;
                    }
                }
                break;
            }
            case '?': {
                err++;
                break;
            }
        }
    }
    if(err) {
        disp_help();
    }
    return 0;
}


