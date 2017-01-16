'''
Name: numsubsequence.py
Date: 2016-12-05
Description: Find the number of subsequences appear in a sequence - using dynamic programming
Reference: http://stackoverflow.com/questions/6877249/find-the-number-of-occurrences-of-a-subsequence-in-a-string
Time differences: 

    for i in range(10000):
        a = num_subsequences('abababababababababab', 'ababaa')
    print("--- %s seconds ---" % (time.time() - start_time))

    Method 1: --- 28.601997137069702 seconds ---
    Method 2: --- 0.968062162399292 seconds ---
    Method 3: --- 0.9851491451263428 seconds ---
    Method 4: --- 0.9229199886322021 seconds ---
    Method 5: --- 0.32222604751586914 seconds ---

Fastest one: Method 5
'''
__author__ = 'Sundong Kim: sundong.kim@kaist.ac.kr'

import time

# straightforward, naïve solution; too slow!

def num_subsequences1(seq, sub):
    if not sub:
        return 1
    elif not seq:
        return 0
    result = num_subsequences1(seq[1:], sub)
    if seq[0] == sub[0]:
        result += num_subsequences1(seq[1:], sub[1:])
    return result

# top-down solution using explicit memoization

def num_subsequences2(seq, sub):
    m, n, cache = len(seq), len(sub), {}
    def count(i, j):
        if j == n:
            return 1
        elif i == m:
            return 0
        k = (i, j)
        if k not in cache:
            cache[k] = count(i+1, j) + (count(i+1, j+1) if seq[i] == sub[j] else 0)
        return cache[k]
    return count(0, 0)

# top-down solution using the lru_cache decorator
# available from functools in python >= 3.2

from functools import lru_cache

def num_subsequences3(seq, sub):
    m, n = len(seq), len(sub)
    @lru_cache(maxsize=None)
    def count(i, j):
        if j == n:
            return 1
        elif i == m:
            return 0
        return count(i+1, j) + (count(i+1, j+1) if seq[i] == sub[j] else 0)
    return count(0, 0)

# bottom-up, dynamic programming solution using a lookup table

def num_subsequences4(seq, sub):
    m, n = len(seq)+1, len(sub)+1
    table = [[0]*n for i in range(m)]
    def count(iseq, isub):
        if not isub:
            return 1
        elif not iseq:
            return 0
        return (table[iseq-1][isub] +
               (table[iseq-1][isub-1] if seq[m-iseq-1] == sub[n-isub-1] else 0))
    for row in range(m):
        for col in range(n):
            table[row][col] = count(row, col)
    return table[m-1][n-1]

# bottom-up, dynamic programming solution using a single array

def num_subsequences5(seq, sub):
    m, n = len(seq), len(sub)
    table = [0] * n
    for i in range(m):
        previous = 1
        for j in range(n):
            current = table[j]
            if seq[i] == sub[j]:
                table[j] += previous
            previous = current
    return table[n-1] if n else 1


if __name__ == '__main__':
    
    start_time = time.time()
    sub = ['out', 'in'] 
    seq = ['out', 'out', 'out', 'in', 'in', '1f']
    a = num_subsequences5(seq, sub)
    print(a)

    

    # for i in range(1000):
    #     a = num_subsequences5('ababababcabacbababababc', 'abcab')

    # print("--- %s seconds ---" % (time.time() - start_time))

  


