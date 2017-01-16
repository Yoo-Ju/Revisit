'''
Name: sequencefeaturegenerator.py
Date: 2017-01-05
Description: Frequent Sequence 를 찾은 후 Feature로 이용하는 모듈 (faster ver)


Input: 
TO DO: 
'''

import timing
import seqmining_sd
import seqmining_sd_taslike
import pandas as pd
import numpy as np
from scipy.special import entr
from collections import defaultdict
import featuregenerator
import preprocessing
import numsubsequence
import useSPMF
import subprocess
import itertools


def is_subseq(x, y):
    it = iter(y)
    return all(c in it for c in x)

## Check this method   
# assert is_subseq('india', 'indonesia')
# assert is_subseq('oman', 'romania')
# assert is_subseq('mali', 'malawi')
# assert is_subseq((''), ('a', 'b', 'c', 'd', 'e'))
# assert not is_subseq('mali', 'banana')
# assert not is_subseq('ais', 'indonesia')
# assert not is_subseq('ca', 'abc')


def entropy(prob1, prob2):
    return(-prob1*np.log2(prob1)-prob2*np.log2(prob2))


def informationGain(a, b, c, d):
    ''' 
    a = (True, 1.0) - Subsequence, Revisit intention      
    b = (True, 0.0)       
    c = (False, 1.0)    
    d = (False, 0.0) 
    '''
    # Entropy before
    prob1a = (a+c) / (a+b+c+d)
    prob2a = 1-prob1a
    entropy_before = entropy(prob1a, prob2a)
    
    # Entropy after sequence
    prob1b = a / (a+b)
    prob2b = 1 - prob1b
    prob3b = c / (c+d)
    prob4b = 1 - prob3b
    
    entropy1 = entropy(prob1b, prob2b)
    entropy2 = entropy(prob3b, prob4b)
    entropy_after = (a+b)/(a+b+c+d)*entropy1 + (c+d)/(a+b+c+d)*entropy2
    
    IG = entropy_before-entropy_after
    
    return IG



def recursivelyFindLongestSequence(aabaaba, new_list):

    try:
        for item in aabaaba:
            testval = 0
            for longt in new_list:
                testval += is_subseq(item, longt)

            if testval == 0:
                new_list.append(item)

        for item in new_list:
            aabaaba.remove(item)


        recursivelyFindLongestSequence(aabaaba, new_list)
    except:
        pass


### If multiple subsequences have same support, there is a chance to have closed sequence and its subsequence
### Here, we only select independent longest sequences by deleting its subs.
### Technique used: Chance key, value and removing subs by dynamic programming
### Method recursivelyFindLongestSequence is used for the dynamic programming part.
### Input: freq_seqs_sample - Output: freqfreqfreq

def leavelongest_samesupport(freq_seqs_sample):

    freq_seqs_sample2 = {}
    for kv in freq_seqs_sample:
        freq_seqs_sample2.setdefault((kv[1], kv[2]), []).append(kv[0])

    freqfreqfreq = []


    for k, v in freq_seqs_sample2.items():
        if len(v) > 1:
            v = sorted(v, key = len, reverse=True)
            new_list = []
            new_list.append(tuple(v[0]))
            
            recursivelyFindLongestSequence(v, new_list) 
            for item in new_list:
                freqfreqfreq.append(tuple((item, k)))
        else:
            freqfreqfreq.append(tuple((tuple(v[0]), k)))

    freqfreqfreq = sorted(freqfreqfreq, key=lambda tup: tup[1], reverse=True)

    return freqfreqfreq




def generate_sortE(df, supportRatio):

    ### Extract subsequences with support greater than supportRatio
    seqs = df.apply(lambda x: reindexing(x['traj'], x['ts'], x['ts_end'], x['dwell_time'], x['revisit_intention'], x['date_device_id']), axis=1) 
    freq_seqs = seqmining_sd_taslike.freq_seq_enum(seqs, seqs.shape[0]*supportRatio)

    ### Minimum subsequence length == 1  &  sort by support (TODO: parameter)
    freq_seqs_sample = []
    for x in freq_seqs:
        if (len(x[0]) >= 4):
            freq_seqs_sample.append(x)
    freq_seqs_sample = sorted(freq_seqs_sample, key=lambda tup: tup[1], reverse=True)   

    frequency_dict = defaultdict(dict)
    for i in freq_seqs_sample:
        frequency_dict[tuple(i[0])] = i[-1]  ## element 길이가 1인 걸 tuple화하면 다음과 같이 변한다 -> tuple([('zero', 'out-medium')]) = (('zero', 'out-medium'),)

 
    ### Leave closed sequential pattern 
    longest_sequences_support = leavelongest_samesupport(freq_seqs_sample)
    
    print("Number of TAS-like frequent sequences having support larger than threshold : ",len(freq_seqs_sample))
    print("Number of TAS-like frequent closed sequences having support larger than threshold :", len(longest_sequences_support))
    
    num1 = df.revisit_intention.value_counts().loc[1]
    num0 = df.revisit_intention.value_counts().loc[0]
    

    ### Calculate information gain on-the-way
    ''' 
    a = (True, 1.0) - Have subsequence(True), Willing to revisit(1.0)     
    b = (True, 0.0)       
    c = (False, 1.0)    
    d = (False, 0.0) 
    '''
    longest_sequences_support2 = []
    
    for i in longest_sequences_support:
        z = []  
        a = i[1][1]  
        b = i[1][0] - i[1][1]
        c = num1 - a
        d = num0 - b
        z.append(a)
        z.append(b)
        z.append(c)
        z.append(d)
        ig = informationGain(a, b, c, d)
        z.append(ig)
        longest_sequences_support2.append((i[0], z))

    igdict = {}  
    for traj in longest_sequences_support2:
        igdict[tuple(traj[0])] = traj[1]

    sortE = sorted(igdict.items(), key=lambda value: value[1][-1], reverse=True)
    return sortE, frequency_dict



def generate_seqE(sortE, numFeatures):
    seqE = []
    for item in sortE[:numFeatures]:
        seqE.append(item[0])
    return seqE


def reindexing(q, a, b, dtime, idx, p):
    startmp = a[0]
    newa = [x-startmp for x in a]
    newb = [x-startmp for x in b]
    return [[q, newa, newb, dtime, idx, p]]

def reindexing2(q, a, b, dtime, p):
    startmp = a[0]
    newa = [x-startmp for x in a]
    newb = [x-startmp for x in b]
    return [[q, newa, newb, dtime, p]]

def num_subseqqq(seq, sub):
    
    m, n = len(seq[0][0]), len(sub)
    explore_cand = {}
    
    ## TAS pattern에 속하는 area들이 moving pattern에 있나 없나 확인 - True면 계속 진행 - 개수 확인
    if (is_subseq([item[1] for item in sub], seq[0][0]) == True):
        for j in range(n):
            if(len(explore_cand.keys()) == 0):
                for i in range(m):
                    if((sub[j][1] == seq[0][0][i]) and (sub[j][0] == seqmining_sd_taslike.temporalinterval(seq[0][1][i]-0))):
                        explore_cand[i] = 1
                         
            else:
                explore_cand2 = explore_cand
                explore_cand = {}
                for cand_idx in explore_cand2.keys():
                    i = cand_idx+1
                    while(i < m):
                        time_previousarea = seq[0][1][cand_idx]
                        if((sub[j][1] == seq[0][0][i]) and (sub[j][0] == seqmining_sd_taslike.temporalinterval(seq[0][1][i]-time_previousarea))):
                            if i not in explore_cand:
                                explore_cand[i] = explore_cand2[cand_idx]
                            else:
                                explore_cand[i] += explore_cand2[cand_idx]
                        i += 1
        return sum(explore_cand.values())
                   
    else:
        return 0
    
# seq = [[['out-long', 'in-medium', '1f-right-medium',
#       '2f-right-medium','in-medium','1f-right-medium','1f-right-medium'],
#       [0, 145, 246, 363, 563, 588, 801],
#       [1805, 982, 644, 604, 644, 981, 966],
#       [1805, 837, 398, 341, 81, 293, 265],
#       0.0]]

# sub = (('medium', 'in-medium'),)
# print(num_subseqqq(seq, sub))







# Top-N TAS-like sequential pattern(seq)을 feature로 이용
# newdf(idx, seq) = Moving Pattern(idx)가 seq를 가진 횟수
def add_features_frequency(df, seqE, fdict, test):   
	
	if(test==True):
		for seq in seqE:
			df[seq] = 0
		df.loc[:, 'seqsinput'] = df.apply(lambda x: reindexing2(x['traj'], x['ts'], x['ts_end'], x['dwell_time'], x['revisit_intention']), axis=1) 
		for seq in seqE:
			df.loc[:, seq] = df['seqsinput'].apply(lambda x: num_subseqqq(x, seq)) 
		del df['seqsinput']
		return df

	elif(test==False):                
	    for seq in seqE:
	        df[seq] = 0
	    for seq in seqE:
	        for idx in fdict[seq].keys():
	            df.set_value(idx, seq, fdict[seq][idx])         
	    return df




@timing.timing            
def add_frequent_sequence_features(df, supportRatio, featureRatio, temporal, test, origin_seqE):
	newdf = df.copy()
	fdict = defaultdict(dict)

	if temporal == True:
	    print('Generating feature: By considering dwell_time of each area')
	    area = preprocessing.getuniqueareas(df.traj)
	    # print(area)
	    newdf.loc[:, 'traj'] = df.apply(lambda x: featuregenerator.add_temporal_sign(x, area), axis=1)
	else:
		print('Generating feature: Not considering dwell_time of each area')
	

	if test == True:
		seqE = origin_seqE
	else:
		print('Case 1: Use bartdag\'s seqmining package with information gain')
		newdf = newdf.reset_index()
		sortE, fdict = generate_sortE(newdf, supportRatio)
		print('sortE calculation done')
		newdf = newdf.set_index(['date_device_id'])
		numFeatures = int(round(newdf.shape[0]*featureRatio))
		seqE = generate_seqE(sortE, numFeatures)
		print()
		print('Top-10 sortE: ', sortE[:10])
		print()
		print('seqE length: ',len(seqE))

		## Case 2 (use SPMF - can be used SPMF-generated subsequences as features)
		# areadict = useSPMF.encryptAreaSPMFconvertible(df.traj)
		# f = open('spmftestsample.txt', 'w')
		# useSPMF.toSPMFconverter(df.traj, areadict, f)
		# subprocess.check_output(['java', '-jar', 'spmf.jar', 'run', 'VMSP', 'spmftestsample.txt', 'spmftestsampleoutput2.txt', '2%'])
		# traj_support = useSPMF.readSPMFresult('spmftestsampleoutput2.txt', areadict)
		# seqE = [item[0] for item in traj_support]

	newdf = add_features_frequency(newdf, seqE, fdict, test)

	    ## 지금 상황: TAS-like sequential patterns을 prefixScan 형식으로 incremental하게 구하는 동시에, 
	    ## 각 moving pattern별로 해당 TAS-like sequential pattern들이 몇 번 포함되어 있는 지 카운트가 되었고, 
	    ## Sequential pattern들을 feature로 사용할 때 기 카운트한 정보를 입력하도록 코드가 짜여 있었다.
	    ## 문제는, 10-fold cross-validation에서 train data를 이용하여 TAS-like sequential pattern들을 찾고,
	    ## test set에서는 해당 sequential patterns들이 각 moving pattern별로 몇 번 들어가있나 계산해야 하는데
	    ## 현재 코드는 sequential pattern을 찾으면서 찾아지는 pattern들의 빈도를 동시에 카운트 하는 구조라, 
	    ## 기존 training data에서 찾아진 sequential patterns들이 test set의 moving pattern들에 몇 번씩 
	    ## 들어가 있는 지 알 수 없다... 따라서 numsubsequence.num_subsequences5(mp, seq) 같은 카운트 모듈을 
	    ## 다시 짜야 할 것 같다.... 

	oldtonew_columnnames = {}
	val = 10001
	for i in newdf.columns:
	    if(isinstance(i, tuple)):
	        oldtonew_columnnames[i] = val
	        val += 1
	newdf = newdf.rename(columns=oldtonew_columnnames)   
	  
	return newdf, seqE


if __name__ == '__main__':
	print("--------------TEST sequencefeaturegenerator_taslike.py-------------")
	df = pd.read_pickle("../data/786/786_mpframe3.p")
	print(df.shape)
	df2 = preprocessing.remove_frequent_visitors(df, 90, 10)
	print(df2.shape)
	finaldf, seqE= add_frequent_sequence_features(df2, 0.02, 0.02, True, False, [])
	print(finaldf.shape)
	for i in range(100):
		print(i, "--", list(finaldf.columns)[i+50])
		print(finaldf[list(finaldf.columns)[i+50]].value_counts())





