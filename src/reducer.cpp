#include "reducer.h"

void *reducer(void *arg)
{
    reducer_arg *reducer_arg = (struct reducer_arg *)arg;
    int id = reducer_arg->id;
    int reducer_count = reducer_arg->reducer_count;
    pthread_barrier_t *barrier = reducer_arg->barrier;
    std::vector<std::set<std::pair<std::string, int>>> *mapper_results = reducer_arg->mapper_results;
    std::vector<std::pair<std::string, int>> *shared_data = reducer_arg->shared_data;
    pthread_mutex_t *shared_data_mutex = reducer_arg->shared_data_mutex;
    pthread_barrier_t *shared_data_barrier = reducer_arg->shared_data_barrier;
    std::map<std::string, std::set<int>> *buckets = reducer_arg->buckets;
    pthread_mutex_t *buckets_mutex = reducer_arg->buckets_mutex;
    

    // Wait for all the mappers to finish
    pthread_barrier_wait(barrier);
    printf("Reducer %d started\n", id);

    // Concatenate all the words from the mappers
    int start = mapper_results->size() * id / reducer_count;
    int end = mapper_results->size() * (id + 1) / reducer_count;

    printf("Reducer %d processing word blocks from %d to %d\n", id, start, end);
    for (int i = start; i < end; i++) 
    {
        pthread_mutex_lock(shared_data_mutex);
        for (auto word : mapper_results->at(i)) 
        {
            shared_data->push_back(word);
        }
        pthread_mutex_unlock(shared_data_mutex);
    }

    // Sync the threads
    pthread_barrier_wait(shared_data_barrier);

    start = shared_data->size() * id / reducer_count;
    end = shared_data->size() * (id + 1) / reducer_count;

    // Sort the words into buckets
    printf("Reducer %d processing words from %d to %d\n", id, start, end);
    for (int i = start; i < end; i++) 
    {
        std::pair<std::string, int> item = shared_data->at(i);
        std::string word = item.first;
        int file_id = item.second;
        int char_index = word[0] - 'a';

        pthread_mutex_lock(&buckets_mutex[char_index]);
        buckets[char_index][word].insert(file_id);

        pthread_mutex_unlock(&buckets_mutex[char_index]);
    }

    // Sync the threads
    pthread_barrier_wait(shared_data_barrier);
    
    char start_char = 'a' + id * MAX_CHARS / reducer_count;
    char end_char = 'a' + (id + 1) * MAX_CHARS / reducer_count;

    // Write the output to files
    printf("Reducer %d processing words from %c to %c\n", id, start_char, end_char - 1);
    for (char c = start_char; c < end_char; c++) 
    {
        int char_index = c - 'a';
        std::vector<std::pair<std::string, std::set<int>>> output(buckets[char_index].begin(), buckets[char_index].end());

        // Sort the output
        std::sort(output.begin(), output.end(), 
        [](const std::pair<std::string, std::set<int>> &a, const std::pair<std::string, std::set<int>> &b) {
            if (a.second.size() == b.second.size()) {
                return a.first < b.first;
            }
            return a.second.size() > b.second.size();
        });

        std::ofstream output_file(c + std::string(".txt"));
        for (auto item : output) 
        {
            output_file << item.first << ":[";
            for (auto file_id : item.second) 
            {

                output_file << file_id;
                if (file_id != *item.second.rbegin())
                    output_file << " ";
            }
            output_file << "]\n";
        }
    }

    return NULL;
}