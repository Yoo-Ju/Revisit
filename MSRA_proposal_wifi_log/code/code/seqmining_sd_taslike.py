### TAS-like sequence 찾는 코드
from collections import defaultdict


def freq_seq_enum(sequences, min_support):
    '''Enumerates all frequent sequences.
       :param sequences: A sequence of sequences.
       :param min_support: The minimal support of a set to be included.
       :rtype: A set of (frequent_sequence, support).
    '''
    freq_seqs = []
    includingSeqs = defaultdict(dict)
    _freq_seq(sequences, [], 0, 0, min_support, freq_seqs, includingSeqs)
    return freq_seqs


def _freq_seq(sdb, prefix, prefix_support, revisit_support, min_support, freq_seqs, includingSeqs):
    if prefix:
        freq_seqs.append([prefix, prefix_support, revisit_support, includingSeqs[prefix[-1]]])
        includingSeqs = defaultdict(dict)
    locally_frequents = _local_freq_items(sdb, min_support, True, includingSeqs)
    if not locally_frequents:
        return
    for (item, support1, support2, includingSeqs) in locally_frequents:
        new_prefix = prefix + [item,]
        new_sdb = _project(sdb, new_prefix)
        _freq_seq(new_sdb, new_prefix, support1, support2, min_support, freq_seqs, includingSeqs)


## With temporal annotation (temporal=True)
def _local_freq_items(sdb, min_support, temporal, includingSeqs):
    items = defaultdict(int)  ## for support
    items2 = defaultdict(int)  ## for revisit_intention_support
    freq_items = []
    if temporal == True:
        for sequence in sdb:
            visited = set()
            for entry in sequence: 
                for i in range(len(entry[0])):
                    element = (temporalinterval(entry[1][i]), entry[0][i])
                    includingSeqs[element][entry[-1]] = includingSeqs[element].get(entry[-1], 0) + 1
                    if element not in visited:
                        items[element] += 1
                        items2[element] += int(entry[-2])
                        visited.add(element)
    if temporal == False:
        for sequence in sdb:
            visited = set()
            for entry in sequence:
                for i in range(len(entry[0])):
                    element = entry[0][i]
                    includingSeqs[element][entry[-1]] = includingSeqs[element].get(entry[-1], 0) + 1
                    if element not in visited: 
                        items[element] += 1
                        items2[element] += int(entry[-2])
                        visited.add(element)  
    for item in items:
        support = items[item] ## support
        support2 = items2[item] ## revisit_intention support
        if support >= min_support:
            freq_items.append([item, support, support2, includingSeqs]) 
    return freq_items


def _project(sdb, prefix):
    new_sdb = []
    if not prefix:
        return sdb
    current_prefix_item = prefix[-1]
    for sequence in sdb:
        lol = []
        for entry in sequence:
            for j in range(len(entry[0])):
                projection = None
                if entry[0][j] == current_prefix_item[-1]:  
                    if temporalinterval(entry[1][j]) == current_prefix_item[0]:
                        projection = [entry[0][j + 1:], [x-entry[1][j] for x in entry[1][j + 1:]], [x-entry[1][j] for x in entry[2][j + 1:]], entry[-2], entry[-1]] 
                        lol.append(projection)
        if lol:
            new_sdb.append(lol)
    return new_sdb


def temporalinterval(num):
    res = 0
    if num == 0:
        res = 'zero'
    elif num < 10:
        res = 'veryshort'
    elif num < 100:
        res = 'short'
    elif num < 1000:
        res = 'medium'
    elif num > 1000:
        res = 'long'
    return res


if __name__ == '__main__':
	sdb = [[[['out', 'in', '1f', '1f'], [5, 30, 100, 140], [160, 159, 138, 159], [155, 129, 38, 19], 1, 1]], [[['out', '1f'], [8, 25], [160, 158], [152, 133], 1, 2]], [[['out', '1f', 'in', '1f', 'in', '1f', 'in', '1f', 'in', '2f'], [1, 2, 5, 201, 300, 302, 402, 502, 602, 702], [401, 50, 240, 239, 400, 399, 403, 503, 603, 703], [400, 48, 235, 38, 100, 97, 1, 1, 1, 1], 0, 3]],[[['out', 'in', '1f', '1f'], [3, 30, 100, 140], [160, 159, 138, 159], [157, 129, 38, 19], 1, 4]], [[['out', '1f'], [7, 25], [160, 158], [153, 133], 1, 5]], [[['out', '1f', 'in', '1f', 'in', '1f', 'in', '1f', 'in', '2f'], [0, 2, 5, 201, 300, 302, 402, 502, 602, 702], [401, 50, 240, 239, 400, 399, 403, 503, 603, 703], [401, 48, 235, 38, 100, 97, 1, 1, 1, 1], 0, 6]]]
	print('--------TEST--------')
	print(sdb)
	print()
	print(_project(sdb, [('medium', '1f')]))
	print()
	print(_local_freq_items(sdb, 1, True, defaultdict(dict)))
	print()
	print(freq_seq_enum(sdb, 4))

