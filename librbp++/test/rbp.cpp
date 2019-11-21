#include "run.h"
#include "qrels.h"
#include "rbp.h"

#include <iostream>
#include <fstream>

using namespace rbp;

int main(int argc, char ** argv) {
    System system;
    Qrelset qrelset;

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <qrels> <run>" << std::endl;
        exit(1);
    }
    std::ifstream qin(argv[1]);
    Qrelset::parse_qrels_file(qrelset, qin);
    qin.close();

    std::ifstream rin(argv[2]);
    System::parse_run_file(system, rin);
    rin.close();

    RbpScorer scorer(0.95);
    for (System::ConstIterator it = system.begin(); it != system.end();
      it++) {
        std::string topicid = it->first;
        const Run &run = it->second;
        const Qrels &qrels = qrelset.get(topicid);
        RbpScore score = scorer.score(qrels, run);
        std::cout << topicid << " " << score.get_base()
            << " +" << score.get_residual() << std::endl;
    }
    return 0;
}
