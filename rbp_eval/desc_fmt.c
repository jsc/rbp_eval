#include "config.h"
#include "desc_fmt.h"
#include "error.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#ifdef HAVE_OPENSSL_MD5_H
#include <openssl/md5.h>
#endif /* HAVE_OPENSSL_MD5_H */

static void desc_fmt_qid(qid_res_t * qres, depth_t * d_spec,
  persist_t * p_spec, FILE * fp); 

static void desc_header(fmt_args_t * args, FILE * fp);

void desc_fmt(res_t * res, fmt_args_t * args, FILE * fp) {
    unsigned q;
    enum details_t details = args->details;

    if (!args->opt->no_header)
        desc_header(args, fp);

    for (q = 0; q < res->num_qid; q++) {
        qid_res_t * qres;

        qres = &res->qid_res[q];
        if (qres->num_rel < 0) {
            warning("there are no judgments for query id %s", qres->qid);
            continue;
        }
        if (details & DETAILS_PER_QUERY) {
            desc_fmt_qid(qres, res->depth, res->persist, fp);
        }
    }
    if (details & DETAILS_AVERAGES) {
        desc_fmt_qid(&res->ave_res, res->depth, res->persist, fp);
    }
}

#define DEPTH_BUF_SIZE 32

static void desc_fmt_qid(qid_res_t * qres, depth_t * d_spec,
  persist_t * p_spec, FILE * fp) {
    unsigned d, p;
    for (d = 0; d < d_spec->d_num; d++) {
        unsigned depth = d_spec->d[d];
        char depth_buf[DEPTH_BUF_SIZE];
        depth_res_t * dres = &qres->depth_res[d];

        if (depth == DEPTH_FULL) {
            strcpy(depth_buf, "full");
        } else {
            snprintf(depth_buf, DEPTH_BUF_SIZE, "%u", depth);
        }
        for (p = 0; p < p_spec->p_num; p++) {
            double persist = p_spec->p[p];
            persist_res_t * pres = &dres->persist_res[p];

            fprintf(fp, "p= %.2lf q= %4s d= %4s rbp= %.4f +%.4f\n",
              persist, qres->qid, depth_buf, pres->sum, pres->err);
        }
    }
}

#define HOSTNAME_BUF_SIZE 1024
#define CWD_BUF_SIZE 1024

void desc_header(fmt_args_t * args, FILE * fp) {
    int a;
    time_t now;
#ifdef HAVE_GETHOSTNAME
    char hostname_buf[HOSTNAME_BUF_SIZE];
#endif /* HAVE_GETHOSTNAME */
#ifdef HAVE_GETCWD
    char cwd_buf[CWD_BUF_SIZE];
#endif /* HAVE_GETCWD */
#if defined(HAVE_GETUID) && defined(HAVE_GETPWUID)
    struct passwd * pwd;
#endif /* HAVE_GETUID && HAVE_GETPWUID */
    struct stat run_stat;

    now = time(NULL);

    fprintf(fp, "# rbp_eval output\n");
    fprintf(fp, "#\n");
    /* NB unbelievably, ctime() adds a newline to the end of the formatted
     * time. */
    fprintf(fp, "# %s", ctime(&now));
    fprintf(fp, "# command:");
    for (a = 0; a < args->argc; a++) {
        fprintf(fp, " %s", args->argv[a]);
    }
    fprintf(fp, "\n");
#ifdef HAVE_GETHOSTNAME
    gethostname(hostname_buf, HOSTNAME_BUF_SIZE);
    fprintf(fp, "# hostname: %s\n", hostname_buf);
#endif /* HAVE_GETHOSTNAME */
    fprintf(fp, "# rbp_eval version: %s\n", VERSION);
#ifdef HAVE_GETCWD
    getcwd(cwd_buf, CWD_BUF_SIZE);
    fprintf(fp, "# current working directory: %s\n", cwd_buf);
#endif /* HAVE_GETCWD */
#if defined(HAVE_GETUID) && defined(HAVE_GETPWUID)
    pwd = getpwuid(getuid());
    if (pwd != NULL) {
        fprintf(fp, "# user: %s\n", pwd->pw_name);
    }
#endif /* HAVE_GETUID && HAVE_GETPWUID */
    if (stat(args->opt->run_fname, &run_stat) == 0) {
        fprintf(fp, "# %s modification time: %s", args->opt->run_fname,
          ctime(&run_stat.st_mtime));
    }
#ifdef RUN_MD5SUM
    fprintf(fp, "# %s md5sum: %s\n", args->opt->run_fname,
      run_get_md5sum(args->run));
#endif /* HAVE_OPENSSL_MD5_H */
}
