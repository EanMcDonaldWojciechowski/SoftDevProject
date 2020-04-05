#include "../dataframe/modified_dataframe.h"

int main(int argc, char** argv) {
    /**** Static Arguments ***/
    Args* arg = new Args();

    // if user passes in non integers for places where there should be integers
    // just default the value to 0
    arg->parse(argc, argv);


    // opening the file
    // FILE *f = fopen(file_name, "r");
    // affirm(f != NULL, "File is null pointer");
    //
    // SOR* reader = new SOR();
    // reader->read(f, from, len);

    std::cout<<"nodeIndex: " << arg->nodeIndex << "\n";
    std::cout<<"num_nodes: " << arg->num_nodes << "\n";
    std::cout<<"file: " << arg->file << "\n";

    WordCount *wc = new WordCount(arg);
    std::cout<<"Beginning Word Count...\n";
    wc->run_();
    sleep(30);
    // delete reader;
    // fclose(f);
    return 0;
}
