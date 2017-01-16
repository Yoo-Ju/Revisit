'''
Name: main.py
Date: 2016-08-23 ~
Description: Combine everything
Writer: Sundong Kim(sundong.kim@kaist.ac.kr)
'''

import crawler
import reindex
import featuregenerator
import preprocessing
import predict
import sequencefeaturegenerator
import sequencefeaturegenerator_taslike
import pickle
import pandas as pd
import numpy as np
from numpy import inf
import timing2
from sklearn.model_selection import KFold, StratifiedKFold
from sklearn.metrics import accuracy_score
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.gaussian_process.kernels import RBF
from sklearn.preprocessing import normalize
import CompanionTrajectory





pd.options.mode.chained_assignment = None  # make ease to use .loc function 

##################################################
##   Data Directories  
##################################################
placeNum = str(786) 
# crawler.crawling(str(781), str(786))    # Crawler method 

# store_dwell_time_threshold = [0, 1, 10, 100]
# revisit_interval = [7, 30, 90]
# frequent_visitor_threshold = [3, 5, 7, 10, 15]


# store_dwell_time_threshold = [0]
# revisit_interval = [90]
# frequent_visitor_threshold = [5, 7, 10, 15]


rawdata_picklePath = "../data/"+placeNum+"/"+placeNum+"_0_rawdata.p"
df_rd = pd.read_pickle(rawdata_picklePath) 

options = "_0_30_10"
i=0
j=30
k=10

# for i in store_dwell_time_threshold:
# 	for j in revisit_interval:
# 		for k in frequent_visitor_threshold:
			# options = "_"+str(i)+"_"+str(j)+"_"+str(k)
			# print(options)

reindexed_picklePath = "../data/"+placeNum+"/"+placeNum+"_1_reindexed"+options+".p"
trajpreprocessed_picklePath = "../data/"+placeNum+"/"+placeNum+"_2_trajprocessed"+options+".p"
revisitintentionadded_picklePath = "../data/"+placeNum+"/"+placeNum+"_3_revisitintentionadded"+options+".p"
frequentvisitorremoved_picklePath = "../data/"+placeNum+"/"+placeNum+"_4_frequentvisitorremoved"+options+".p"
statistical_picklePath = "../data/"+placeNum+"/"+placeNum+"_5_statistical"+options+".p"
# mpframe6_train_picklePath = "../data/"+placeNum+"/"+placeNum+"_mpframe6_train.p"






##################################################
##  Several Preprocessing steps to make initial train/test data to play
##  (Can be changed tentatively - 170112)
##  1. Reindex raw data, trajectory containing only 'out' are deleted.
##  2. Trajectory preprocessing (Minseok's module)
##  3. Add visit count (Will be ignored), Add revisit count(our objectives)
##  4. Add basic statistical features
##  Detailed description of each methods are written in each script file.   
##################################################

# df_ri = reindex.reindex_by_moving_pattern(df_rd, i) # moving patterns having dwell_time less than 100 in area 'in' are deleted
# df_ri.to_pickle(reindexed_picklePath)
# print('Reindexing process by each indoor moving pattern is done')
# print('Reindexed dataframe is saved in %s' % reindexed_picklePath)


# df_ri = pd.read_pickle(reindexed_picklePath) 
# df_tp = preprocessing.trajectoryPreprocessor(df_ri)
# df_tp.to_pickle(trajpreprocessed_picklePath)
# print('Trajectories are preprocessed by using Minseokkim\'s module')
# print('Revised dataframe with preprocessed trajectories is saved in %s' % trajpreprocessed_picklePath)


# df_tp = pd.read_pickle(trajpreprocessed_picklePath) 
# df_riadded = preprocessing.add_revisit_intention(df_tp, j)   # argv[1] = revisit_interval
# df_riadded.to_pickle(revisitintentionadded_picklePath)
# print('Revisit intention and visit count has been calculated')
# print('Revised dataframe with revisit intention is saved in %s' % revisitintentionadded_picklePath)


# df_riadded = pd.read_pickle(revisitintentionadded_picklePath)
# print('Before removing frequent visitors: ', df_riadded.shape)
# ## remove_frequent_visitors: arg[1]: revisit interval(90days) 이내 방문이 없는 사람들 제거 -- 전 단계와 합쳐야 함, # arg[2]: ignore customers if they visit more than n times(10 times)
# df_fvremoved = preprocessing.remove_frequent_visitors(df_riadded, k)  
# df_fvremoved.to_pickle(frequentvisitorremoved_picklePath)
# print('After removing frequent visitors: ', df_fvremoved.shape)





##################################################
##  Main part of playing with our dataframe (Add features, test accuracy)
##  (Can be changed tentatively - 170112)
##################################################


def check(mpframe3):

	mpframe4 = featuregenerator.add_statistical_features(df_rd, mpframe3)
	mpframe4.to_pickle(statistical_picklePath)
	print('Basic statistical features has been calculated')
	print('Revised dataframe with basic statistical features is saved in %s' % statistical_picklePath)
	mpframe4 = mpframe4.set_index('date_device_id')

	# mpframe3.loc[:, 'revisit_intention'] = mpframe3['revisit_intention'].astype(int)
	# # mask = mpframe3['traj'].str.len() > 4   # Length threshold of moving patterns to use
	# # mpframe3 = mpframe3.loc[mask]
	# print('initial shape of the data frame: ', mpframe3.shape)

	# ## remove_frequent_visitors: arg[1]: revisit interval(90days) 이내 방문이 없는 사람들 제거 -- 전 단계와 합쳐야 함, # arg[2]: ignore customers if they visit more than n times(10 times)
	# mpframe4 = preprocessing.remove_frequent_visitors(mpframe3, 90, 10)  
	# print('After removing frequent visitors: ', mpframe4.shape)

	result4, result5, result6 = [], [], []

	kf = StratifiedKFold(n_splits=10)
	for train_index, test_index in kf.split(mpframe4, mpframe4['revisit_intention']):
		
		## mpframe4: without sequential pattern features
		mpframe4_train = mpframe4.ix[train_index]
		mpframe4_test = mpframe4.ix[test_index]

		## mpframe5: with sequential pattern features
		mpframe5_train, seqE5 = sequencefeaturegenerator.add_frequent_sequence_features(mpframe4_train, 0.005, 0.05, True, False, [])
		mpframe5_test, seqE_deprecated5 = sequencefeaturegenerator.add_frequent_sequence_features(mpframe4_test, 0.005, 0.05, True, True, seqE5) 
		
		## mpframe6: with TAS-like sequential pattern features
		mpframe6_train, seqE6 = sequencefeaturegenerator_taslike.add_frequent_sequence_features(mpframe4_train, 0.005, 0.05, True, False, []) 
		mpframe6_test, seqE_deprecated6 = sequencefeaturegenerator_taslike.add_frequent_sequence_features(mpframe4_test, 0.005, 0.05, True, True, seqE6) 

		print('Frequent sequence features has been added: ', mpframe6_train.shape)	
		print('Frequent sequence features has been added: ', mpframe6_test.shape)


		inputs = [(mpframe4_train, mpframe4_test, result4), (mpframe5_train, mpframe5_test, result5), (mpframe6_train, mpframe6_test, result6)]
		
		for (train_basic,test_basic, result) in inputs:
			train_final = preprocessing.finalprocessing(train_basic)
			test_final = preprocessing.finalprocessing(test_basic)


			train = np.asarray(train_final)
			train[train == inf] = 0
			X_train, y_train = train[:, 12:-1], train[:, -1].astype(int)  ## [:, 11:-1]로 하면 visit_count부터 feature로 사용.
			# X_train = normalize(X_train, norm='l2', axis=1)
			print('Train:', X_train.shape)

			test = np.asarray(test_final)
			test[test == inf] = 0
			X_test, y_test = test[:, 12:-1], test[:, -1].astype(int)
			# X_test = normalize(X_test, norm='l2', axis=1)
			print('Test:', X_test.shape)

			# clf = DecisionTreeClassifier(max_depth=5)
			clf = AdaBoostClassifier()
			clf = clf.fit(X_train, y_train)
			y_pred = clf.predict(X_test)
			prediction_accuracy = accuracy_score(y_test, y_pred)
			result.append(prediction_accuracy)
			# cvresults = predict.basicDecisionTree(X, y)
			print("Result: ", prediction_accuracy)
			# result1.append(np.mean(cvresults))

	print("Average 10-fold accuracy without sequential patterns: ", np.mean(result4))
	print("Average 10-fold accuracy with sequential patterns: ", np.mean(result5))
	print("Average 10-fold accuracy with TAS-sequential patterns: ", np.mean(result6))
	print("------------------")










		# df_learning1_train = preprocessing.finalprocessing(mpframe4_train)
		# df_learning1_test = preprocessing.finalprocessing(mpframe4_test)
		# df_learning3_train = preprocessing.finalprocessing(mpframe6_train)
		# df_learning3_test = preprocessing.finalprocessing(mpframe6_test)


		# train = np.asarray(df_learning1_train)
		# train[train == inf] = 0
		# X_train, y_train = train[:, 12:-1], train[:, -1].astype(int)  ## [:, 11:-1]로 하면 visit_count부터 feature로 사용.
		# # X_train = normalize(X_train, norm='l2', axis=1)
		# print('Train:', X_train.shape)

		# test = np.asarray(df_learning1_test)
		# test[test == inf] = 0
		# X_test, y_test = test[:, 12:-1], test[:, -1].astype(int)
		# # X_test = normalize(X_test, norm='l2', axis=1)
		# print('Test:', X_test.shape)


		# # clf = DecisionTreeClassifier(max_depth=5)
		# clf = AdaBoostClassifier()
		# clf = clf.fit(X_train, y_train)
		# y_pred = clf.predict(X_test)
		# prediction_accuracy = accuracy_score(y_test, y_pred)
		# result1.append(prediction_accuracy)
		# # cvresults = predict.basicDecisionTree(X, y)
		# print("Result 1: ", prediction_accuracy)
		# # result1.append(np.mean(cvresults))


		# train = np.asarray(df_learning3_train)
		# train[train == inf] = 0
		# X_train, y_train = train[:, 11:-1], train[:, -1].astype(int)
		# # X_train = normalize(X_train, norm='l2', axis=0)
		# print('Train(with seq patterns):', X_train.shape)

		# test = np.asarray(df_learning3_test)
		# test[test == inf] = 0
		# X_test, y_test = test[:, 11:-1], test[:, -1].astype(int)
		# # X_test = normalize(X_test, norm='l2', axis=0)
		# print('Test(with seq patterns):', X_test.shape)

		# # clf = DecisionTreeClassifier(max_depth=5)
		# clf = AdaBoostClassifier()
		# clf = clf.fit(X_train, y_train)
		# y_pred = clf.predict(X_test)
		# prediction_accuracy = accuracy_score(y_test, y_pred)
		# result3.append(prediction_accuracy)
		# # cvresults = predict.basicDecisionTree(X, y)
		# print("Result 3: ", prediction_accuracy)
		# # result1.append(np.mean(cvresults))


	# print("Average results for exp 1: ", np.mean(result1))
	# print("Average results for exp 3: ", np.mean(result3))
	# print("-------------")



df_fvremoved = pd.read_pickle(frequentvisitorremoved_picklePath)
check(df_fvremoved)


## mpframe3 = pd.read_pickle(statistical_picklePath)
## mpframe3 = CompanionTrajectory.companionFinder(mpframe3, 3)
## del mpframe3['companion']
## check(mpframe3)
## mpframe3 = pd.read_pickle(statistical_picklePath_beforetrajpreprocess)
## check(mpframe3)




