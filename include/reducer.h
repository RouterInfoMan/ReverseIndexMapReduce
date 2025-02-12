#include <pthread.h>

#include <fstream>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#define MAX_CHARS 26


struct reducer_arg {
    int id; // for debugging
    int reducer_count;

    pthread_barrier_t *barrier;
    std::vector<std::set<std::pair<std::string, int>>> *mapper_results;
    std::vector<std::pair<std::string, int>> *shared_data;
    pthread_mutex_t *shared_data_mutex;
    pthread_barrier_t *shared_data_barrier;

    // buckets
    std::map<std::string, std::set<int>> *buckets;
    pthread_mutex_t *buckets_mutex;
};

void *reducer(void *arg);