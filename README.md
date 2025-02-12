# Reverse Index MapReduce

A multi-threaded implementation of the MapReduce paradigm to generate a reverse index of text files. The program processes multiple input files in parallel, creating a word-to-document index using mapper and reducer threads.

## Compilation and Running

### Prerequisites
- G++ compiler
- POSIX threads support
- Make utility

### Building the Project
```bash
# Clone the repository
git clone https://github.com/RouterInfoMan/ReverseIndexMapReduce.git
cd ReverseIndexMapReduce

# Compile the project
make build

# Clean build files
make clean

# Create distribution archive
make zip
```

### Running the Program
```bash
./ReverseIndexMapReduce <mapper_count> <reducer_count> <input_file>
```

## Architecture

### Map Phase
The mapper threads process input files in parallel, with the following optimizations:
- Files are sorted by size before processing, ensuring larger files are handled first for better load balancing
- Each mapper maintains its own set of word-document pairs to minimize contention
- Thread synchronization is handled through mutexes and barriers

### Reduce Phase
The reducer threads perform a two-stage aggregation process:
1. **Data Consolidation**: Reducers combine mapper results into shared data structure
2. **Bucket Distribution**: Words are distributed into buckets based on their first letter
3. **Final Output**: Each reducer handles a specific range of letters, generating sorted output files

## Features

- Dynamic workload distribution based on file sizes
- Case-insensitive word processing with special character removal
- Efficient thread synchronization using barriers and mutexes
- Sorted output by word frequency and alphabetical order
- Memory-efficient data structures using STL containers

## Usage

### Parameters
- `mapper_count`: Number of parallel mapper threads
- `reducer_count`: Number of parallel reducer threads
- `input_file`: Path to input file containing list of documents to process

### Input Format
The input file should contain:
1. First line: Number of files to process
2. Subsequent lines: One filename per line

### Output
The program generates 26 output files (a.txt through z.txt), each containing words starting with that letter and their document occurrences in the format:
```
word:[doc1 doc2 doc3]
```
Words are sorted by:
1. Frequency (descending)
2. Alphabetically (for words with same frequency)

## Implementation Details

### Mapper Implementation
- Each mapper thread:
  - Retrieves files from a shared queue protected by a mutex
  - Processes words, converting to lowercase and removing special characters
  - Stores results in thread-local sets to avoid contention

### Reducer Implementation
- Three-phase reduction process:
  1. Initial data gathering from mapper results
  2. Word distribution into character-based buckets
  3. Final sorting and output generation
- Uses multiple synchronization barriers to ensure proper phase transitions
- Efficient memory management using STL containers (maps, sets, vectors)

## Performance Considerations

- Files are processed largest-first to optimize thread utilization
- Minimal lock contention through careful synchronization design
- Memory-efficient data structures with STL containers
- Balanced workload distribution among reducer threads

## Dependencies

- C++11 or higher
- POSIX threads (pthread)
- Standard Template Library (STL)

## Project Structure
```
ReverseIndexMapReduce/
├── src/
│   ├── main.cpp
│   ├── mapper.cpp
│   └── reducer.cpp
├── include/
│   ├── mapper.h
│   └── reducer.h
├── Makefile
└── README.md
```