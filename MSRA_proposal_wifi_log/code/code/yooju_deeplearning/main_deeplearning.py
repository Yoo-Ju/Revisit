'''
Name: main_deeplearning.py
Date: 2016-11-08
Description: Deep learning prototype using Keras library (Backend: Tensorflow)
To-Do: 


Background: 
* Why keras?

* Why RNN & LSTM? 블로그 포스트들과 우리 연구와의 관련성
	* http://karpathy.github.io/2015/05/21/rnn-effectiveness/
	    * Sequences input -> Positive or negative output도 처리 가능
	    * 예제: Minimal character level RNN language model in python/numpy:
	    https://gist.github.com/karpathy/d4dee566867f8291f086
	* http://colah.github.io/posts/2015-08-Understanding-LSTMs/

* 평가 방법 관련
	* http://machinelearningmastery.com/evaluate-performance-deep-learning-models-keras/
'''

import pandas as pd
import pickle
import itertools
import math
from keras.models import Sequential
from keras.layers import LSTM, Dense
from sklearn.model_selection import train_test_split
import numpy as np
import preprocessing


def getuniqueareas(trajseries):
    aggregated_traj = list(itertools.chain.from_iterable(trajseries))
    uniqueareas = sorted(list(set(aggregated_traj)))
    return uniqueareas


# print(str(df3.traj).value_count())
def embedding(x, areas, areavecdict):
    datalen = len(x.traj)
    twod_result = np.zeros((10, 1447)) 
    for idx in range(datalen):
#         print(idx)
#         print(twod_result.shape[0])
        if(idx < twod_result.shape[0]):
            
            area_name = x.traj[idx]
            area_num = areas.index(area_name)
            area_vec = areavecdict.get(area_name)
            
#             area_vec = np.equal.outer(area_num, np.arange(len(areas))).astype(np.float)
#             print(area_num)
#             print(area_vec)

            starttime_name = x.ts[idx]
#             starttime_num = int(starttime_name[:starttime_name.index( ':')]) 
            starttime_num = int(starttime_name)
            snum_refine = (starttime_num /60 % 1440 + 540) % 1440
            
    
#             starttime_name = x.time_start[idx]

#             starttime_vec = np.equal.outer(starttime_num, np.arange(24)).astype(np.float)
            
    #         print(starttime_name)
    #         print(starttime_num)
    #         print(starttime_vec)
    
            endtime_name = x.ts_end[idx]
            endtime_num = int(endtime_name)
            enum_refine = (endtime_num /60 % 1440 + 540) % 1440
            
            sr = math.floor(snum_refine)
            er = math.ceil(enum_refine)
            
            dt_vec = np.zeros(1440)
            for i in range(sr, er):
                dt_vec[i] = 1

#             dwelltime_num = int(x.dwell_time[idx])
#             print('d', dwelltime_num % 60)

            total_vec = np.append(area_vec, dt_vec)
#             total_vec = np.append(total_vec, dt_vec)
#             print(total_vec)
            
            twod_result[idx, :] = total_vec
    
    return twod_result



def dataGenerator(df2):
	df3 = df2   ## .head(1000)
	areas = getuniqueareas(df3.traj)
	length = df3.shape[0]
	steps = 10
	dim = 1447
	x = np.zeros((length, steps, dim))
	idx = 0

	areavecdict = {}
	areavecdict['out'] = np.array([0, 0, 0, 0, 0, 0, 0])
	areavecdict['in'] = np.array([1, 0, 0, 0, 0, 0, 0])
	areavecdict['1f'] = np.array([1, 1, 0, 0, 0, 0, 0])
	areavecdict['2f'] = np.array([1, 0, 1, 0, 0, 0, 0])
	areavecdict['3f'] = np.array([1, 0, 0, 1, 0, 0, 0])
	areavecdict['1f-inner'] = np.array([1, 1, 0, 0, 1, 0, 0])
	areavecdict['1f-left'] = np.array([1, 1, 0, 0, 0, 1, 0])
	areavecdict['1f-right'] = np.array([1, 1, 0, 0, 0, 0, 1])
	areavecdict['2f-inner'] = np.array([1, 0, 1, 0, 1, 0, 0])
	areavecdict['2f-left'] = np.array([1, 0, 1, 0, 0, 1, 0])
	areavecdict['2f-right'] = np.array([1, 0, 1, 0, 0, 0, 1])

	for row in df3.itertuples():  
	#         print(len(row.traj))
	    rowrst = embedding(row, areas, areavecdict)
	    x[idx, :, :] = rowrst
	    idx += 1

	y = np.asarray(pd.get_dummies(df3['revisit_intention']))


	# x_train = x[:7500, :, :]
	# x_val = x[7500:,:, :]
	# y_train = y[:7500, :]
	# y_val = y[7500:, :]
	return x, y  
    









if __name__ == '__main__':

	placeNum = str(786)
	# statistical_picklePath2 = "../code/data/"+placeNum+"/"+placeNum+"_mpframe3.p"
	statistical_picklePath = "./"+placeNum+"_mpframe3.p"
	df = pd.read_pickle(statistical_picklePath)
	print(df.shape)
	
	df = preprocessing.remove_frequent_visitors(df, 90, 10)
	
	df2 = df[['traj', 'ts', 'dwell_time', 'time_start', 'ts_end', 'revisit_intention']]  #  'time_end',
	
	areas = getuniqueareas(df2.traj)
	x, y = dataGenerator(df2)

	x_train, x_val, y_train, y_val = train_test_split(x, y, test_size=0.2)


	######## KERAS code from here.

	data_dim = x_val.shape[2]
	timesteps = x_val.shape[1]
	nb_classes = 2

	# def modelGenerator():
	model = Sequential()
	model.add(LSTM(32, return_sequences=True,
	               input_shape=(timesteps, data_dim)))  # returns a sequence of vectors of dimension 32
	model.add(LSTM(32, return_sequences=True))  # returns a sequence of vectors of dimension 32
	model.add(LSTM(32))  # return a single vector of dimension 32
	model.add(Dense(2, activation='softmax'))

	model.compile(loss='binary_crossentropy',
	              optimizer='rmsprop',
	              metrics=['accuracy'])

	# # generate dummy training data
	# x_train = np.random.random((1000, timesteps, data_dim))
	# y_train = np.random.random((1000, nb_classes))

	# # generate dummy validation data
	# x_val = np.random.random((100, timesteps, data_dim))
	# y_val = np.random.random((100, nb_classes))

	model.fit(x_train, y_train,
	          batch_size=64, nb_epoch=20,
	          validation_data=(x_val, y_val))

	# print(df3.ix[39])
	# print('-----')
	# print(embedding(df3.ix[1], areas))
