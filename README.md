# Protocol for `myfind` Program

## Objective

The `myfind` program is designed to search for specified files within a directory structure, with optional recursive and case-insensitive search modes. The program is implemented in C, using process-based parallelism to achieve concurrent file searches.

## Key Concepts in Parallelization and Output Synchronization

### 1. Process-based Parallelism

- Each filename search request is handled in parallel by creating a separate child process using `fork()`. This allows multiple file searches to be executed concurrently, making the program faster than a serial approach, especially when searching multiple files in large directory structures.
- Each child process is responsible for searching a specific filename within the specified `searchpath`.
- Recursive subdirectory searches are also handled in parallel by forking additional child processes within the `search_directory` function whenever a directory is encountered and the `-R` (recursive) option is active.

### 2. Argument Parsing

- Command-line arguments are processed using the `getopt()` function, allowing flexibility in the order and inclusion of optional flags `-R` (recursive search) and `-i` (case-insensitive search).
- The program can accept a variable number of filenames, processing each in a separate child process.

### 3. Output Synchronization

- Each child process independently outputs the search results for its assigned filename in the format:
  <pid>: <filename>: <complete-path-to-found-file>

- To avoid output conflicts (when multiple processes write to `stdout` at the same time), the program relies on the system's buffered output to ensure that each line is written in a single operation. This simplifies the need for explicit synchronization methods like mutexes, which are generally more suitable for threads within a single process.

### 4. Zombie Process Prevention

- After each child process completes its search, it is crucial to prevent it from becoming a zombie (an orphaned process that remains in the system). This is handled by having the parent process wait for each child process to terminate using `wait()` within a loop.
- This ensures that the program releases resources associated with terminated child processes promptly, maintaining system performance and resource efficiency.

### 5. Case-Insensitive Matching

- The `-i` option enables case-insensitive searches by converting both the target filename and the current file name to lowercase before comparison.
- This is implemented with helper functions to handle string transformations for case-insensitivity.

### 6. Recursive Search Implementation

- When the `-R` option is specified, the program will search not only in the specified directory but also in all subdirectories.
- Each time a subdirectory is encountered, the program forks a new child process to handle the search in that directory, allowing the search to occur in parallel across multiple directory levels.

---

This protocol provides an overview of the `myfind` program's structure, focusing on concurrency, argument parsing, and effective handling of output synchronization.
