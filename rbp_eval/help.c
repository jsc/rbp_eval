#include "help.h"

static const char * usage_fmt = "USAGE: %s [options] <qrels-file> <run-file>\n";
static const char * options_str= "options:\n"
"   -d DEPTH_SPEC    ranking depths to calculate rbp to.  A comma-separated\n"
"                      list of positive integers, or 0 to indicate to\n"
"                      calculate for all documents in the run.\n"
"   -p PERSIST_SPEC  user persistences to calculate rbp with.  A \n"
"                      comma-separated list of floats in range [0.0,1.0]\n"
"   -q               print rbp values for each query (default is only give\n"
"                      the overall averages)\n"
"   -T               do not print overall averages ('T'otals)\n"
"   -o               rank documents by order of occurence in the run file.\n"
"   -r               rank documents by the rankings given in column 4 of\n"
"                      the run file.  Two documents with the same ranking\n"
"                      will be considered equal for rbp calculation\n"
"   -s               rank documents by scores given in column 5 of the run\n"
"                      file.  Two document with the same score will be \n"
"                      considered equal for rbp calculation.  This is the\n"
"                      default ranking.\n"
"   -b THRESHOLD     convert relevance judgments in qrels file to binary\n"
"                      at threshold THRESHOLD.\n"
"   -B               convert relevance judgments in qrels file to binary\n"
"                      at threshold 1.\n"
"   -f SCALE         retain fractional relevance judgments in qrels file,\n"
"                      multiplying all relevance judgments by SCALE.\n"
"   -F               retain fractional relevance judgments in qrels file,\n"
"                      without scaling.\n"
"   -a               automatically determine relevance treatment based on\n"
"                      contents of qrels file.  If it contains only integral\n"
"                      relevances, treat as binary at threshold 1\n"
"                      (as with the -B option); if it contains\n"
"                      fractional relevances, treat as fractional without\n"
"                      scaling (as with the -F option).\n"
"   -H               do not add header comment to output.\n"
"   -W               suppress warning messages.\n"
"   -h               this help message\n";

static const char * long_help = "";

void print_help(char * progname, FILE * stream) {
    fprintf(stream, usage_fmt, progname);
    fputs(options_str, stream);
    fputs(long_help, stream);
}
