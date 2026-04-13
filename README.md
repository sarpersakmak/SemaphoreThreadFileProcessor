# SemaphoreThreadFileProcessor

**CMPE 382 Operating Systems — Homework 2**  
**Synchronization: Semaphores and Critical Section Implementation**  
**Student:** Sarper Sakmak | **ID:** 14008175400 | **Section:** 5

---

## Description

A multi-threaded C program that uses POSIX threads and a counting semaphore
to process multiple text files concurrently. The program counts deficient
numbers in each file while limiting the number of simultaneously active
threads to a user-specified value.

A positive integer n is **deficient** if the sum of its proper divisors
is strictly less than n (e.g. 8 → 1+2+4 = 7 < 8 ✓).

---

## How It Works

- Main thread scans the given directory for `.txt` files
- For each file, main calls `sem_wait()` before spawning a thread
- Each worker thread counts deficient numbers, prints the result, then calls `sem_post()`
- Never more than `threadNumber` threads are active at the same time
- Uses **detached threads** — no `pthread_join()` needed

This mirrors the classic office-hours analogy: the office fits at most
`n` students; a new student enters only when one leaves.

---

## Usage

### 1. Generate test files
```bash
chmod +x new.sh
./new.sh
```

### 2. Compile
```bash
gcc -o hw2 14008175400_hw2.c -lpthread
```

### 3. Run
```bash
./hw2 <directoryName> <threadNumber>
```

**Example:**
```bash
./hw2 myDir 3
```

### 4. Measure execution time
```bash
time ./hw2 myDir 1
time ./hw2 myDir 2
...
time ./hw2 myDir 8
```

---

## Sample Output
