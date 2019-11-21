#include "run.h"

#include <sstream>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define FIELD_NUM 6

//#define original_code
#define updated_code

std::string string_to_upper(char str[]);
char *strip_zeros(char word[], int len);
char *strip_zeros2(char word[], int len);
void remove_commas(char word[], int len, char *new_word);

using namespace rbp;

bool RunDoc::parse_run_line(RunDoc & rd, std::string & topicid_out,
  std::string & sysid_out,
  std::istream & sin, unsigned occur) throw (ParseException) {
    char buf[max_run_line_len_ + 1];

    sin.getline(buf, max_run_line_len_ + 1);
    if (sin.gcount() == max_run_line_len_ + 1) {
        throw ParseException("Line longer than max run line length");
    } else if (sin.eof()) {
        return false;
    } else if (!sin.good()) {
        throw ParseException("Error in input stream");
    }
    // NOTE: 60% of the time of parsing a run file is taken up
    // by the rest of this function.  There would probably be
    // a speed up in parsing the line 'by hand', and not using
    // std::string or std::istringstream here. 
    
    #ifdef updated_code
    char word[max_run_line_len_ + 1], word3[max_run_line_len_ + 1];
    char *word2, *wordstop;
    std::string docid;
    int i = 0, wordi = 0, isword = 0, wordcount = 0;
    unsigned long rank;
    double score;
        
    for (i = 0; buf[i] != '\0' && (wordcount < FIELD_NUM); i++) {
        if (!isspace(buf[i])) {
                word[wordi] = buf[i];
                wordi++;
                isword = 1;
        } else {
            if (isword == 1) {
                isword = 0;
                word[wordi] = '\0';
                wordi = 0;
                wordcount++;
                switch (wordcount) {
                case 1 :
                    topicid_out = word;
                    break;
                case 3 :
                    docid = string_to_upper(word);
                    break;
                case 4 :
                    wordstop = NULL;
                    remove_commas(word, strlen(word), word3);
                    word2 = strip_zeros(word3, strlen(word3));
                    rank = strtoul(word2, &wordstop, 0);
                    if (!((strcmp(word2, "") != 0) &&
                          (strcmp(wordstop, "") == 0))) {
                        throw ParseException("Incorrect format for rank\nInvalid run line: '" + std::string(buf) + "'");
                        free(word2);
                        free(wordstop);
                    }
                    break;
                case 5 :
                    wordstop = NULL;
                    score = strtod(word, &wordstop);
                    if (!((strcmp(word, "") != 0) &&
                          (strcmp(wordstop, "") == 0))) {
                        throw ParseException("Incorrect format for score\nInvalid run line: '" + std::string(buf) + "'");
                        free(wordstop);
                    }
                    break;
                }
            }
        }
    }
    // To prevent overwriting the last word
    // This is executed if the end of the buffer is reached and the last 
    // character was not a space.
    if( wordi != 0 ) {
        word[wordi] = '\0';
        wordcount++;
    }
    sysid_out = word;

    // This takes care of any spaces on the end of the line.
    //if (strcmp(word, "") != 0) {
    //    wordcount++;
    //}

    if (wordcount == 0) {
        throw ParseException("Blank Line\nInvalid run line: '" + std::string(buf) + "'");
    } else if (wordcount < FIELD_NUM) {
        throw ParseException("Not enough fields on the line\nInvalid run line: '" + std::string(buf) + "'");
    } else if (wordcount > FIELD_NUM) {
        // This code should never be executed as the buffer is trimmed if
        // there are more than 6 fields.
        throw ParseException("Too many fields on the line\nInvalid run line: '" + std::string(buf) + "'");
    }

    rd = RunDoc(docid, occur, rank, score);
    #endif /* updated_code */

    #ifdef original_code
    std::istringstream ss(buf);
    std::string dummy, docid;
    unsigned rank;
    double score;
    ss >> topicid_out >> dummy >> docid >> rank >> score >> sysid_out;
    if (ss.fail() || !ss.eof()) {
        std::cout << topicid_out << " " << dummy << " " << docid << " "
            << rank << " " << score << " " << sysid_out << std::endl;
        throw ParseException("Invalid run line: '" + std::string(buf) + "'");
    }
    rd = RunDoc(docid, occur, rank, score);
    #endif /* original_code */

    return true;
}

/*
 * Function which converts all letters in a string to upper case.
 */
std::string string_to_upper(char str[]) {
    for (unsigned int i = 0 ; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
    return str;
}

void System::dump(std::ostream & out) {
    for (System::Iterator it = runs_.begin(); it != runs_.end(); it++) {
        std::string topicid = it->first;
        Run run = it->second;
        for (Run::Iterator rit = run.begin(); rit != run.end(); rit++) {
            RunDoc doc = *rit;
            out << topicid << "\tQ0\t" << doc.get_docid() << "\t"
                << doc.get_rank() << "\t" << doc.get_score() 
                << "\t" << get_sysid() 
                // << "\t# " << doc.get_occur() 
                <<  std::endl;
        }
    }
}

/*
 * Function which removes commas from a string.
 */
void remove_commas(char word[], int len, char *new_word) {
    for (unsigned i = 0; word[i] != '\0'; i++) {
        if (word[i] != ',') {
            *new_word = word[i];
            new_word += 1;
        }
    }
    *new_word = '\0';
}

/*
 * Function which strips leading zeroes from a given string.
 */
char *strip_zeros(char word[], int len) {
    char *new_word = NULL;

    for (unsigned i = 0 ; word[i] != '\0'; i++) {
        if (word[i] != '0') {
            new_word = &word[i];
            return new_word;
        }
    }
    new_word = "0";
    return new_word;
}

/*
 * Function which strips leading zeroes from a given string.
 */
char *strip_zeros2(char word[], int len) {
    char *new_word;
    new_word = (char*) malloc(len + 1);
    int start = 0, k = 0;
    bool allzero = true;

    // If the string is 0, do not strip characters
    for (unsigned i = 0 ; word[i] != '\0'; i++) {
        if (word[i] != '0') {
            allzero = false;
            break;
        }
    }
    if (allzero) {
        new_word[k] = '0';
        new_word[k+1] = '\0';
        return new_word;
    }

    for (unsigned i = 0 ; word[i] != '\0'; i++) {
        if (word[i] == '0' && start == 0) { 
            // remove leading zeros
        } else {
            // copy remaining elements
            start = 1;
            new_word[k] = word[i];
            k++;
        }
    }
    new_word[k] = '\0';
    return new_word;
}

/*
 * XXX This function also needs to make sure that no document is repeated for the
 * same topic in a file.
 */
void System::parse_run_file(System & sys, std::istream & in) 
    throw (RunDoc::ParseException) {
    RunDoc rd;
    Run run;
    bool first_line = true, var = true;
    unsigned occur = 0;

    std::string last_topicid = "";
    std::string topicid, sysid, allsysid;
    sys.set_error(false);
    while(var == true) {
        try{
            while (var = RunDoc::parse_run_line(rd, topicid, sysid, in)) {
                if (first_line) { 
                    sys.set_sysid(sysid);
                    last_topicid = topicid;
                    first_line = false;
                } else if (topicid != last_topicid) {
                    occur = 0;
                    run.set_topicid(last_topicid);
                    sys.add(last_topicid, run);
                    run.clear();
                    last_topicid = topicid;
                } else {
                    occur++;
                }
                rd.set_occur(occur);
                //std::cout << ".";
                run.append(rd);
            }
        } catch (RunDoc::ParseException e) {
            if (strcmp(e.what(), "Error in input stream")) {
                std::cout << "An exception occurred:\n" << e.what() << "\n\n";
                sys.set_error(true);
                var = false;
            } else {
                std::cout << "An exception occurred:\n" << e.what() << "\n\n";
                sys.set_error(true);
                var = true;
            }
        }
    }

    run.set_topicid(last_topicid);
    sys.add(last_topicid, run);
}
