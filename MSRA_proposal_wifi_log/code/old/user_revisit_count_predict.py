__author__ = 'ZFTurbo: https://kaggle.com/zfturbo'
import datetime
import pandas as pd
import numpy as np
from sklearn.cross_validation import train_test_split
import xgboost as xgb
import random
import zipfile
import time
import shutil
from sklearn.metrics import log_loss
from sklearn.metrics import mean_squared_error
import json

def feature_generator(df_toy):
	print('Generating features from raw data')
	### F1: 로그 총 개수
	f1 = df_toy.groupby(['device_id'])['ts'].count()

	### F2: 와이파이에 잡힌 총 시간
	f2 = df_toy.groupby(['device_id'])['dwell_time'].sum()

	### F3: dwell_time > 100인 indoor area 개수
	df_toy_indoor = df_toy.loc[df_toy['area']!='out']
	df_toy_indoor2 = df_toy_indoor.loc[df_toy_indoor['dwell_time']>100]
	f3 = df_toy_indoor2.groupby(['device_id'])['area'].count()

	### F4: indoor 로그 중 dwell_time > 100인 확률
	f3_2 = df_toy_indoor.groupby(['device_id'])['area'].count()
	f4 = f3.div(f3_2)

	### F5: deny = True일 확률
	a = df_toy.groupby(['device_id']).deny.count()
	b = df_toy['device_id'].value_counts()
	f5 = a.div(b)

	### F6: dwell_time > 100인 indoor area에서 보낸 total time
	f6 = df_toy_indoor2.groupby(['device_id'])['dwell_time'].sum()

	### F7: dwell_time > 100인 indoor area들의 variance
	f7 = df_toy_indoor2.groupby(['device_id'])['dwell_time'].std()

	### F8: 로그 총 개수 - 요일별
	days = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
	f8 = df_toy_indoor2.groupby(['day', 'device_id'])['dwell_time'].sum()
	f8 = f8.reindex(days, level='day')
	f8 = f8.to_frame(name='count').reset_index()
    
	### Label: Maximum revisit count from the log
	label_toy = df_toy.groupby(['device_id'])['revisit_count'].max()

	return f1, f2, f3, f4, f5, f6, f7, f8, label_toy

def df_generator(df, f1, f2, f3, f4, f5, f6, f7, f8, label):
	print('Generating a data frame which aggergated features')

	days = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
	days_numlogs = ['num_logs_' + s for s in days]
	columns = ['num_logs', 'total_dwell_time', 'num_sp_100', 'prob_dwell_10', 'prob_deny', 'time_sp_100', 'std_sp_100']
	columns = columns + days_numlogs

	# feature들과의 index의 통일을 위해 np.sort를 이용.
	device_ids = np.sort(df['device_id'].unique())       
	df2 = pd.DataFrame(columns=columns, index=device_ids)

	# feature를 df에 삽입
	df2["num_logs"] = f1          
	df2["total_dwell_time"] = f2
	df2["num_sp_100"] = f3
	df2["prob_dwell_10"] = f4
	df2["prob_deny"] = f5
	df2["time_sp_100"] = f6
	df2["std_sp_100"] = f7

	### F8를 df에 합치는 부분
	for day in days:
	    f8_certain_day = f8.loc[f8['day']==day]
	    f8_certain_day = f8_certain_day[["device_id", "count"]].set_index(['device_id'])
	    columnName = 'num_logs'+day
	    df2[columnName] = f8_certain_day

	# label을 df에 합침
	df2 = pd.concat([df2, label], axis=1)   

	# machine learning에 바로 이용될 dataframe을 리턴
	return df2


def run_xgb(train, test, features, target, random_state=0):
	start_time = time.time()
	objective = "reg:linear"
	booster = "gbtree"
	eval_metric = ["auc", "rmse"]
	eta = 0.1
	max_depth = 3
	subsample = 0.7
	colsample_bytree = 0.7
	silent = 1

	print('XGBoost params. ETA: {}, MAX_DEPTH: {}, SUBSAMPLE: {}, COLSAMPLE_BY_TREE: {}'.format(eta, max_depth, subsample, colsample_bytree))
	params = {
		"objective": objective,
#         "num_class": 2,
		"booster" : booster,
		"eval_metric": eval_metric,
		"eta": eta,
		"max_depth": max_depth,
		"subsample": subsample,
		"colsample_bytree": colsample_bytree,
		"silent": silent,
		"seed": random_state,
	}
	num_boost_round = 200
	early_stopping_rounds = 20
	test_size = 0.2

	X_train, X_valid = train_test_split(train, test_size=test_size, random_state=random_state)
	print('Length train:', len(X_train.index))
	print('Length valid:', len(X_valid.index))
	y_train = X_train[target]
	y_valid = X_valid[target]
	dtrain = xgb.DMatrix(X_train[features], y_train)
	dvalid = xgb.DMatrix(X_valid[features], y_valid)

	watchlist = [(dtrain, 'train'), (dvalid, 'eval')]
	gbm = xgb.train(params, dtrain, num_boost_round, evals=watchlist, early_stopping_rounds=early_stopping_rounds, verbose_eval=True)

	print("Validating...")
	check = gbm.predict(xgb.DMatrix(X_valid[features]), ntree_limit=gbm.best_iteration)

	score = mean_squared_error(y_valid.tolist(), check)

	print("Predict test set...")
	test_prediction = gbm.predict(xgb.DMatrix(test[features]), ntree_limit=gbm.best_iteration)

	training_time = round((time.time() - start_time)/60, 2)
	print('Training time: {} minutes'.format(training_time))

	print(gbm)
    
    # To save logs
	explog = {}
	explog['features'] = features
	explog['target'] = target
	explog['params'] = {}
	explog['params']['objective'] = objective
	explog['params']['booster'] = booster
	explog['params']['eval_metric'] = eval_metric
	explog['params']['eta'] = eta
	explog['params']['max_depth'] = max_depth
	explog['params']['subsample'] = subsample
	explog['params']['colsample_bytree'] = colsample_bytree
	explog['params']['silent'] = silent
	explog['params']['seed'] = random_state
	explog['params']['num_boost_round'] = num_boost_round
	explog['params']['early_stopping_rounds'] = early_stopping_rounds
	explog['params']['test_size'] = test_size
	explog['length_train']= len(X_train.index)
	explog['length_valid']= len(X_valid.index)
	# explog['gbm_best_iteration']= 
	explog['score'] = score
	explog['training_time'] = training_time


	
    
	return test_prediction.tolist(), score, explog


def updateLog(explog, logPath):
	try:
	    with open(logPath, 'r') as f:
	        obob = json.load(f)
	    f.close()
	except:
	    obob = []

	    
	obob.append(explog)

	with open(logPath, 'w') as f:
	    json.dump(obob, f)
	f.close()




random.seed(2016)
datadir = "../data/781/781.p"

df = pd.read_pickle(datadir)

remainder = (df['ts']%604800000)/1000

def timestamp_to_day(x):
	a = x / 86400
	switcher = {
	    0: "Thu",
	    1: "Fri",
	    2: "Sat",
	    3: "Sun",
	    4: "Mon",
	    5: "Tue",
	    6: "Wed"
	}
	return switcher.get(int(a))


df['day'] = remainder.apply(lambda x: timestamp_to_day(x))

f1, f2, f3, f4 ,f5, f6, f7, f8, label = feature_generator(df)
df2 = df_generator(df, f1, f2, f3, f4, f5, f6, f7, f8, label)
df2 = df2.fillna(0)
df2 = df2.reindex(np.random.permutation(df2.index))

idx = int(len(df2.index)*9/10)
train = df2[:idx]
test = df2[idx:]
features = list(df2.columns)[:-1]
target = 'revisit_count'

print('Length of train: ', len(train))
print('Length of test: ', len(test))
print('Features [{}]: {}'.format(len(features), sorted(features)))

test_prediction, score, explog = run_xgb(train, test, features, target)
print('Score: ', score)

logPath = '../result/results.json'

explog['dataset']= datadir
explog['ts']= time.strftime('%Y-%m-%d %H:%M:%S')

updateLog(explog, logPath)














