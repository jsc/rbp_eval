#include "opt.h"
#include "error.h"
#include "help.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

void opt_error(char * fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

void opt_init(struct opt * opt) {
    opt->depth.d_num = 0;
    opt->persist.p_num = 0;
    opt->ord = -1;
    /* because the detail field is a bitmask of flags, we have to
     * set it its default values here. */
    opt->details = DETAILS_AVERAGES;
    opt->reltype = -1;
    opt->reltype_arg = -1.0;
    opt->help_and_exit = -1;
    opt->no_header = -1;
    opt->no_warnings = -1;
}

void opt_set_defaults(struct opt * opt) {
    if (opt->depth.d_num == 0)
        parse_depth(&opt->depth, DEFAULT_DEPTH, NULL, 0);
    if (opt->persist.p_num == 0)
        parse_persist(&opt->persist, DEFAULT_PERSIST, NULL, 0);
    if (opt->ord == -1)
        opt->ord = QDOCS_DEFAULT_ORDERING;
    /* See opt_init for treatment of opt->details. */
    if (opt->help_and_exit == -1)
        opt->help_and_exit = 0;
    if (opt->reltype == -1)
        opt->reltype = RELTYPE_AUTO;
    if (opt->reltype_arg == -1.0)
        opt->reltype_arg = 1.0;
    if (opt->no_header == -1)
        opt->no_header = 0;
    if (opt->no_warnings == -1)
        opt->no_warnings = 0;
}

#define ERR_BUF_LEN 1024

int opt_getopt(struct opt * opt, int argc, char * const argv[]) {
    const char * optstring = "aBb:Ff:d:p:qTrshHoW";
    int optflag;
    int error = 0;
    char err_buf[ERR_BUF_LEN];

    while ( (optflag = getopt(argc, argv, optstring)) != -1) {
        switch (optflag) {
        case 'd':
            if (opt->depth.d_num != 0) {
                opt_error("depth spec (-d) already given");
                error = 1;
            } else {
                if (parse_depth(&opt->depth, optarg, err_buf, ERR_BUF_LEN) 
                  < 0) {
                    opt_error(err_buf);
                    error = 1;
                } else if (opt->depth.d_num == 0) {
                    opt_error("empty depth specification");
                    error = 1;
                }
            }
            break;
        case 'p':
            if (opt->persist.p_num != 0) {
                opt_error("persist spec (-p) already given");
                error = 1;
            } else {
                if (parse_persist(&opt->persist, optarg, err_buf, ERR_BUF_LEN) 
                  < 0) {
                    opt_error(err_buf);
                    error = 1;
                } else if (opt->persist.p_num == 0) {
                    opt_error("empty persist specification");
                    error = 1;
                }
            }
            break;
        case 'q':
            if (opt->details & DETAILS_PER_QUERY) {
                opt_error("per-query output (-q) already specified");
                error = 1;
            } else {
                opt->details |= DETAILS_PER_QUERY;
            }
            break;
        case 'T':
            if (!opt->details & DETAILS_AVERAGES) {
                opt_error("averages output already suppressed (-T)");
                error = 1;
            } else {
                opt->details ^= DETAILS_AVERAGES;
            }
            break;
        case 'r':
            if (opt->ord != -1) {
                opt_error("ranking (-r, -s, -o) already specified");
                error = 1;
            } else {
                opt->ord = QDOCS_ORD_RANK;
            }
            break;
        case 's':
            if (opt->ord != -1) {
                opt_error("ranking (-r, -s, -o) already specified");
                error = 1;
            } else {
                opt->ord = QDOCS_ORD_SCORE;
            }
            break;
        case 'o':
            if (opt->ord != -1) {
                opt_error("ranking (-r, -s, -o) already specified");
                error = 1;
            } else {
                opt->ord = QDOCS_ORD_OCCUR;
            }
            break;
        case 'b':
            if (opt->reltype != -1) {
                opt_error("relevance type (-b, -B, -f, -F, -a) already "
                  "specified");
                error = 1;
            } else {
                char * endptr;
                double val;
                val = strtod(optarg, &endptr);
                if (*endptr != '\0' || val < 0.0) {
                    opt_error("invalid argument to -b option");
                    error = 1;
                } else {
                    opt->reltype = RELTYPE_BINARY;
                    opt->reltype_arg = val;
                }
            }
            break;
        case 'B':
            if (opt->reltype != -1) {
                opt_error("relevance type (-b, -B, -f, -F, -a) already "
                  "specified");
                error = 1;
            } else {
                opt->reltype = RELTYPE_BINARY;
            }
            break;
        case 'f':
            if (opt->reltype != -1) {
                opt_error("relevance type (-b, -B, -f, -F, -a) already "
                  "specified");
                error = 1;
            } else {
                char * endptr;
                double val;
                val = strtod(optarg, &endptr);
                if (*endptr != '\0' || val <= 0.0) {
                    opt_error("invalid argument to -f option");
                    error = 1;
                } else {
                    opt->reltype = RELTYPE_FRACT;
                    opt->reltype_arg = val;
                }
            }
            break;
        case 'F':
            if (opt->reltype != -1) {
                opt_error("relevance type (-b, -B, -f, -F, -a) already "
                  "specified");
                error = 1;
            } else {
                opt->reltype = RELTYPE_FRACT;
            }
            break;
        case 'a':
            if (opt->reltype != -1) {
                opt_error("relevance type (-b, -B, -f, -F, -a) already "
                  "specified");
                error = 1;
            } else {
                opt->reltype = RELTYPE_AUTO;
            }
            break;
        case 'H':
            if (opt->no_header != -1) {
                opt_error("no-header option (-H) already specified");
                error = 1;
            } else {
                opt->no_header = 1;
            }
            break;
        case 'W':
            if (opt->no_warnings != -1) {
                opt_error("no-warning option (-W) already specified");
                error = 1;
            } else {
                opt->no_warnings = 1;
            }
            break;
        case 'h':
            opt->help_and_exit = 1;
            break;
        case '?':
            opt_error("unknown option '%c'", optflag);
            break;
        }
    }
    if (opt->help_and_exit != 1) {
        if (optind > argc - 2) {
            opt_error("both qrels and run files must be specified");
            error = 1;
        } else if (optind < argc - 2) {
            opt_error("trailing arguments");
            error = 1;
        } else {
            opt->qrels_fname = argv[optind];
            opt->run_fname = argv[optind + 1];
        }
    }
    if (error)
        return -1;
    else 
        return 0;
}

int opt_process(struct opt * opt, int argc, char * const argv[]) {
    int ret;
    opt_init(opt);
    ret = opt_getopt(opt, argc, argv);
    if (ret != 0) {
        print_help(argv[0], stderr);
        return -1;
    } else {
        opt_set_defaults(opt);
        if (opt->help_and_exit) {
            print_help(argv[0], stdout);
            return 0;
        }
    }
    return 1;
}

#ifdef OPT_MAIN

int main(int argc, char ** argv) {
    int ret;
    struct opt opt;

    opt_init(&opt);
    ret = opt_getopt(&opt, argc, argv);
    if (ret != 0) {
        print_help(argv[0], stderr);
    } else {
        opt_set_defaults(&opt);
        if (opt.help_and_exit)
            print_help(argv[0], stdout);
    }
    return ret;
}


#endif /* OPT_MAIN */
