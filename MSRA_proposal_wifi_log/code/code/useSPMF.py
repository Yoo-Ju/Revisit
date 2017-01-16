'''
Name: useSPMF.py
Date: 2016-12-05
Description: Python script to use SPMF open source software

현재 걸리는 시간: 
10만개 Moving pattern에 1785개 Subsequence가 몇 번 쓰였는지 확인하는 데 걸린 시간 (num_subsequences5)
Calculating numsubsequence for 1000 moving patterns: 1338.4238109588623 seconds ---

10만개 Moving pattern에 1785개 Subsequence가 쓰였는지의 유무를 확인하는 데 걸린 시간 (is_subseq)
Calculating numsubsequence for 1000 moving patterns: 242.14750599861145 seconds ---
'''


import pickle
import pandas as pd
import subprocess
import itertools
import sequencefeaturegenerator
import numsubsequence
import time


def getuniqueareas(trajs):
    aggregated_traj = list(itertools.chain.from_iterable(trajs))
    uniqueareas = sorted(list(set(aggregated_traj)))
    return uniqueareas


def encryptAreaSPMFconvertible(trajs):
	areadict = {}
	arealist = getuniqueareas(trajs)
	i = 1
	for item in arealist:
	    areadict[item] = i
	    i += 1
	return areadict


def toSPMFconverter(trajs, areadict, f):
	num = 0
	for i, item in enumerate(trajs):  
	    if(i > 0):
	        f.write('\n')
	    num += 1
	    item2 = []
	    for area in item:
	        item2.append(areadict[area])
	        item2.append(-1)
	    item2.append(-2)
	    eachline = ' '.join(map(str, item2))
	    f.write(eachline)
	f.close()


# def spmftoDF():


def readSPMFresult(path, areadict):
	tuples_traj_support = []

	areadict_reversed = {str(y):x for x,y in areadict.items()}

	with open(path) as f:
		for line in f:
			words = line.split()
			words = [area for area in words if area != '-1']
			traj = [areadict_reversed[area] for area in words[:-2]]
			
			tupp = (traj, int(words[-1]))
			tuples_traj_support.append(tupp)

	f.close()

	return tuples_traj_support



	



#### DONE
## SPMF에서 나온 결과물을 읽어들인다.
## 읽어들인 trajectory를 다시 복호화해야 하는데 -- 복호화하려면 toSPMFconverter에서 딕셔너리 형식으로 넘겨줘야 할 것 같다.
## 몇번 등장했나 카운트하는 메소드 완료

#### HAVEN'T DONE
## Feature로 빠르게 쓰려면 복호화할 필요는 없고 몇 번 등장했나 카운트만 하면 되지만, 이해할 수 있으려면 복호화해야 함.
## data-frame으로 다시 만든 후 몇 번 등장했나 카운트한다.

if __name__ == '__main__':
	
	df = pd.read_pickle('../data/786/786_mpframe3.p')
	print(df.traj.head(3))
	areadict = encryptAreaSPMFconvertible(df.traj)
	# f = open('spmftestsample.txt', 'w')
	# toSPMFconverter(df.traj, areadict, f)
	# subprocess.check_output(['java', '-jar', 'spmf.jar', 'run', 'VMSP', 'spmftestsample.txt', 'spmftestsampleoutput.txt', '2%'])
	
	## spmfToDF()
	traj_support = readSPMFresult('spmftestsampleoutput.txt', areadict)


	start_time = time.time()
	for seq in df.traj:
		for atuple in traj_support:
			sub = atuple[0]
			# numsubsequence.num_subsequences5(seq, sub)
			sequencefeaturegenerator.is_subseq(sub, seq)


	print("--- Calculating numsubsequence for 1000 moving patterns: %s seconds ---" % (time.time() - start_time))		


			# print('seq: ', seq, ', sub: ', sub)
			# print(numsubsequence.num_subsequences5(seq, sub))

	# sub = ['out', 'in']     ##  traj_support[0][0]
	# seq = df.traj[0]
	# print('seq: ', seq, ', sub: ', sub)
	# print(numsubsequence.num_subsequences5(seq, sub))
	


