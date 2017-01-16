### 이 코드에서 계산중에 같이 collect할 수 있을거라 생각했는데 , 포기.
from collections import defaultdict
import timing

@timing.timing
def freq_seq_enum(sequences, min_support):
    '''Enumerates all frequent sequences.

       :param sequences: A sequence of sequences.
       :param min_support: The minimal support of a set to be included.
       :rtype: A set of (frequent_sequence, support).
    '''
    freq_seqs = set()
    _freq_seq(sequences, tuple(), 0, 0, min_support, freq_seqs)
    return freq_seqs


def _freq_seq(sdb, prefix, prefix_support, revisit_support, min_support, freq_seqs):
    if prefix:
#         print('prefix: yes', prefix)
        freq_seqs.add((prefix, prefix_support, revisit_support))
#         print('freq seqs', freq_seqs)
    locally_frequents = _local_freq_items(sdb, prefix, min_support)
#     print('locally frequents', locally_frequents)
    if not locally_frequents:
#         print ('not locally frequents')
        return
    for (item, support1, support2) in locally_frequents:
        new_prefix = prefix + (item,)
        new_sdb = _project(sdb, new_prefix)
#         print('new_prefix', new_prefix)
#         print('new_sdb', new_sdb)
        _freq_seq(new_sdb, new_prefix, support1, support2, min_support, freq_seqs)


def _local_freq_items(sdb, prefix, min_support):
    items = defaultdict(int)  ## for support
    items2 = defaultdict(int)  ## for revisit_intention_support
    freq_items = []
    for entry in sdb:
        visited = set()
        for element in entry[0]:
#             print (element)
            if element not in visited:
                items[element] += 1
                items2[element] += entry[1]
                visited.add(element)
    # Sorted is optional. Just useful for debugging for now.
    for item in items:
        support = items[item] ## support
        support2 = items2[item] ## revisit_intention
        if support >= min_support:
            freq_items.append((item, support, support2))
    return freq_items


def _project(sdb, prefix):
    new_sdb = []
    if not prefix:
        return sdb
    current_prefix_item = prefix[-1]
    for entry in sdb:
        j = 0
        projection = None
        for item in entry[0]:
            if item == current_prefix_item:
                projection = (entry[0][j + 1:], entry[1])
                break
            j += 1
        if projection:
            new_sdb.append(projection)
    return new_sdb



