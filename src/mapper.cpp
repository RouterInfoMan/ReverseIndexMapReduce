#include "mapper.h"


std::string clean_word(const std::string &word)
{
    std::string cleaned_word;
    cleaned_word.reserve(word.size());
    for (char ch : word)
    {
        if (isalpha(ch)) 
        {
            cleaned_word += tolower(ch);
        }
    }
    return cleaned_word;
}

void* mapper(void *arg) 
{
    mapper_arg *mapper_arg = (struct mapper_arg *)arg;
    int id = mapper_arg->id;
    std::queue<std::pair<std::string, int>> *files = mapper_arg->files;
    pthread_mutex_t *files_mutex = mapper_arg->files_mutex;
    std::vector<std::set<std::pair<std::string, int>>> *mapper_results = mapper_arg->mapper_results;
    pthread_barrier_t *barrier = mapper_arg->barrier;

    printf("Mapper %d started\n", id);


    // Take a file and process it
    while (true) 
    {
        std::string filename;
        int file_id;

        // take a file any file
        pthread_mutex_lock(files_mutex);
        if (files->empty())
        {
            pthread_mutex_unlock(files_mutex);
            break;
        }
        filename = files->front().first;
        file_id = files->front().second;
        files->pop();
        printf("Mapper %d processing file %s\n", mapper_arg->id, filename.c_str());
        pthread_mutex_unlock(files_mutex);

        std::ifstream file(filename);
        // read file and separate words
        std::string word;
        while (file >> word)
        {
            std::string cleaned_word = clean_word(word);
            if (cleaned_word.size() > 0) 
            {
                mapper_results->at(id).insert(std::make_pair(cleaned_word, file_id));
            }
        }
    }

    pthread_barrier_wait(barrier);
    return NULL;
}
