# SemaphoreThreadFileProcessor

**CMPE 382 – Operating Systems | Homework 2**
**Topic:** Synchronization with Semaphores & Critical Sections
**Student:** Sarper Sakmak
**ID:** 14008175400
**Section:** 5

---

## Overview

This project is a multi-threaded C application that processes multiple text files concurrently using **POSIX threads (pthreads)** and a **counting semaphore**.

The main goal is to safely control access to a shared resource (thread execution) and ensure that no more than a specified number of threads run at the same time.

Each thread reads a file and counts how many numbers inside it are **deficient numbers**.

---

## What is a Deficient Number?

A positive integer **n** is called *deficient* if the sum of its proper divisors is strictly less than the number itself.

**Example:**

```
8 → 1 + 2 + 4 = 7  <  8  → Deficient
```

---

## Program Workflow

1. The main thread scans the given directory for `.txt` files
2. Before creating a new thread, it calls `sem_wait()`
3. A worker thread is created to process the file
4. The worker:

   * Reads numbers from the file
   * Counts how many are deficient
   * Prints the result
   * Calls `sem_post()` when finished
5. This ensures that at most **N threads** run concurrently

---

## Key Concepts Used

* POSIX Threads (`pthread`)
* Counting Semaphores
* Critical Section control
* Concurrent file processing
* Detached threads (`pthread_detach`)

---

## Why Semaphores?

Semaphores are used to limit concurrency.

Think of it like a room that can hold only **N people**:

* A thread can enter only if there is space (`sem_wait`)
* When it leaves, it signals availability (`sem_post`)

This prevents resource overuse and keeps execution controlled.

---

## Compilation & Execution

### 1. Generate Test Files

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

---

## Performance Testing

You can observe the effect of different thread limits using:

```bash
time ./hw2 myDir 1
time ./hw2 myDir 2
time ./hw2 myDir 4
time ./hw2 myDir 8
```

---

## Sample Output

```
file1.txt → 128 deficient numbers
file2.txt → 95 deficient numbers
file3.txt → 143 deficient numbers
```

---

## Notes

* Threads are created as **detached**, so `pthread_join()` is not required
* The program focuses on synchronization rather than maximum parallelism
* Output order may vary due to concurrent execution

---

## Summary

This project demonstrates how semaphores can be used to:

* Control the number of active threads
* Protect critical sections
* Safely manage concurrent workloads

It provides a clear example of synchronization in real-world multi-threaded systems.
