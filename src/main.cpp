#include "mapper.h"
#include "reducer.h"


#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <queue>
#include <set>
#include <map>

int main(int argc, char **argv)
{
    // argv[1] mapper count
    // argv[2] reducer count
    // argv[3] input file

    // Check if the number of arguments is correct
    if (argc != 4)
    {
        printf("Usage: %s <mapper_count> <reducer_count> <input_file>\n", argv[0]);
        return 1;
    }

    // Get the number of mappers and reducers
    int mapper_count = atoi(argv[1]);
    int reducer_count = atoi(argv[2]);

    assert(mapper_count > 0);
    assert(reducer_count > 0);

    std::queue<std::pair<std::string, int>> files;
    std::vector<std::tuple<std::string, int, int>> _files;

    // Get the input file
    int file_count = 0;
    std::ifstream input(argv[3]);
    input >> file_count;
    
    // Read files and sort by the filesize
    std::string filename;
    for (int i = 0; i < file_count; i++)
    {
        input >> filename;
        // get file size
        FILE *file = fopen(filename.c_str(), "r");
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fclose(file);
        _files.push_back(std::make_tuple(filename, i + 1, size));
    }
    // Sort files by size
    std::sort(_files.begin(), _files.end(), 
    [](const std::tuple<std::string, int, int> &a, const std::tuple<std::string, int, int> &b) {
        return std::get<2>(a) >std::get<2>(b);
    });

    for (auto file : _files)
        files.push(std::make_pair(std::get<0>(file), std::get<1>(file)));

    // Create the mappers
    pthread_t mappers[mapper_count];
    pthread_t reducers[reducer_count];

    // Create the synchronization primitives
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, mapper_count + reducer_count);

    pthread_mutex_t files_mutex;
    pthread_mutex_init(&files_mutex, NULL);

    // Mapper results
    std::vector<std::set<std::pair<std::string, int>>> mapper_results;
    mapper_results.resize(mapper_count);
    
    // Create the mapper threads
    mapper_arg mapper_args[mapper_count];
    for (int i = 0; i < mapper_count; i++)
    {
        mapper_args[i].id = i;
        mapper_args[i].mapper_count = mapper_count;
        mapper_args[i].files = &files;
        mapper_args[i].files_mutex = &files_mutex;
        mapper_args[i].mapper_results = &mapper_results;
        mapper_args[i].barrier = &barrier;
        pthread_create(&mappers[i], NULL, mapper, &mapper_args[i]);
    }

    // shared data
    std::vector<std::pair<std::string, int>> shared_data;
    pthread_mutex_t shared_data_mutex;
    pthread_mutex_init(&shared_data_mutex, NULL);

    // buckets
    std::map<std::string, std::set<int>> buckets[MAX_CHARS];
    pthread_mutex_t buckets_mutex[MAX_CHARS];
    for (int i = 0; i < MAX_CHARS; i++)
    {
        pthread_mutex_init(&buckets_mutex[i], NULL);
    }
    pthread_barrier_t shared_data_barrier;
    pthread_barrier_init(&shared_data_barrier, NULL, reducer_count);

    // Create the reducer threads
    reducer_arg reducer_args[reducer_count];
    for (int i = 0; i < reducer_count; i++)
    {
        reducer_args[i].id = i;
        reducer_args[i].reducer_count = reducer_count;
        reducer_args[i].barrier = &barrier;
        reducer_args[i].mapper_results = &mapper_results;
        reducer_args[i].shared_data = &shared_data;
        reducer_args[i].shared_data_mutex = &shared_data_mutex;
        reducer_args[i].buckets = buckets;
        reducer_args[i].buckets_mutex = buckets_mutex;
        reducer_args[i].shared_data_barrier = &shared_data_barrier;
        pthread_create(&reducers[i], NULL, reducer, &reducer_args[i]);
    }

    for (int i = 0; i < mapper_count; i++)
    {
        pthread_join(mappers[i], NULL);
    }
    for (int i = 0; i < reducer_count; i++)
    {
        pthread_join(reducers[i], NULL);
    }
}