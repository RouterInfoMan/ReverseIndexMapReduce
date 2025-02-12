#include <pthread.h>

#include <fstream>
#include <queue>
#include <string>
#include <map>
#include <set>


struct mapper_arg {
    int id;
    int mapper_count;

    std::queue<std::pair<std::string, int>> *files;
    pthread_mutex_t *files_mutex;
    std::vector<std::set<std::pair<std::string, int>>> *mapper_results;
    
    pthread_barrier_t *barrier;
};

void *mapper(void *arg);