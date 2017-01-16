'''
Name: preprocessing.py
Date: 2016-11-02
Description: trajectoryPreprocessor(by Minseok kim, minseokkim@kaist.ac.kr), and Several preprocessing module(by Sundong Kim)

Input: 
TO DO: trajectoryPreprocessing method: Optimization in terms of speed (takes 15s for 1000 rows now.)
'''
__author__ = 'Sundong Kim: sundong.kim@kaist.ac.kr'

import timing
import pandas as pd
import numpy as np
import itertools
import time
import datetime
import re
import random

########################
## Floor Finding Part ##
########################

def floorFinder(dev_data):
    floors = []
    p = re.compile("^.*-") # Regular expression to find out specific floor data
    for x in dev_data['traj']:
        # if specific floor data exists
        if p.findall(x):
            # get floor of specific floor is.
            if p.findall(x)[0] not in floors:
                # Put floor excluding '-' into floors list
                floors.append(p.findall(x)[0][:-1])
    
    return floors # returns specific floors

########################
## Floor Erasing part ##
########################

def floorEraser(dev_data, floors):
    indexer = []
    # Get floor indices of floors which has specific floors
    for idx, trajs in enumerate(dev_data['traj']):
        if trajs in floors:
            indexer.append(idx)
        # Parts where indexing 'in' and 'out' to erase.
        #-* To Erase them, remove '#' below two lines. *-#
        # if idx > 1 and trajs in ['in', 'out']:
        #    indexer.append(idx)

    # Erase floors where specific floors are
    if indexer :
        for idx, val in enumerate(dev_data):
            dev_data[idx] = [i for j, i in enumerate(val) if j not in indexer]

    return dev_data

#########################
## Floor Creating Part ##
#########################

def floorCreator(dev_data):
    
    # non_floor = ['in', 'out']
    f_dict = {'traj' : '', 'ts' : '', 'ts_end' : ''}
    first = True
    new_floors = {'traj' : [], 'ts' : [], 'ts_end' : []}
    # new_floors = []
    
    for idx, floor_data in enumerate(dev_data['traj']):
        # For only specific floors
        if len(floor_data) > 3:
            # if specificFloorFinder(floor_data) not in non_floor:
            # if floor_data is not 'out', 'in'
            
            if specificFloorFinder(floor_data) == specificFloorFinder(dev_data['traj'][idx-1]):
                # if current floor is same as last floor
                continue
            
            floorTimeSetter(f_dict, dev_data[['traj', 'ts', 'ts_end']], idx) # initialize floor
            first = False
            
            if idx == (len(dev_data['traj']) -1 ):
                # if last of the trajectory then save and break
                for item in f_dict:
                    new_floors[item].append(f_dict[item])
                break
            
            
            next_f_data = dev_data['traj'][idx+1] # next floor data
            
            if len(next_f_data) < 4:
                # if next floor data is 'out' or 'in' then continue
                for item in f_dict:
                    new_floors[item].append(f_dict[item])
                continue
    
            if specificFloorFinder(floor_data) != specificFloorFinder(next_f_data):
                # when two data are diffrent floor
                
                if dev_data['ts'][idx+1] < dev_data['ts_end'][idx]:
                    # next s_floor start before this s_floor end?
                    if dev_data['ts_end'][idx+1] < dev_data['ts_end'][idx]:
                        # case 1 : ex - 1f-I.time_start < 2f-I.time_start && 1f-I.time_end > 2f-I.time_end
                        # set next s.floor start time as floor end time
                        f_dict['ts_end'] = dev_data['ts'][idx+1]
                    else:
                        # case 3 : ex - 1f-I.time_start < 2f-I.time_start && 1f-I.time_end < 2f-I.time_end
                        # set next s.floor start time as floor end time
                        f_dict['ts_end'] = dev_data['ts'][idx+1]
                else:
                    # case 2 : ex - 1f-I.time_end < 2f-I.time_start
                    # set next s.floor start time as floor end time
                    f_dict['ts_end'] = dev_data['ts_end'][idx]
            else:
                # case 4 : same floor data.
                # when same floor data are in a row
                # set next s.floor end time as floor end time continuously
                i = 1
                while ((idx + i) <= len(dev_data['traj']) - 1):
                    if specificFloorFinder(floor_data) != specificFloorFinder(dev_data['traj'][idx+i]):
                        # in the case that two floor are not same
                        break
                    else:
                        f_dict['ts_end'] = dev_data['ts_end'][idx+1]
                        i = i + 1
            
            # put newly calculated floor into set
            for item in f_dict:
                new_floors[item].append(f_dict[item])
        #else:
    # if floor_data is 'out' or 'in'
    # go for next loop


    # return the result
    return new_floors

def floorTimeSetter(f_dict, dev_data, idx):
    f_dict['traj'] = specificFloorFinder(dev_data['traj'][idx]) # set floor
    f_dict['ts'] = dev_data['ts'][idx] # set start time
    f_dict['ts_end'] = dev_data['ts_end'][idx] # set end time

def floorTimeComparator(time1, time2):
    # Return true when time2 is later than time1
    return True if (time2 - time1) > 0 else False

def specificFloorFinder(floor_data): # return floor
    p = re.compile("^.*-")
    if p.findall(floor_data):
        # return floor data before '-' : left, 1f, 2f, etc. Anything before '-'
        return p.findall(floor_data)[0][:-1]
    else :
        # return data if floor_data is not specific floor data
        return floor_data

##########################
## Floor Inserting Part ##
##########################

def floorInsert(dev_data, newfloors):
    val_last = ''
    for idx, val in enumerate(dev_data['ts']):
        if val_last == val: # Pass when value is same as last one
            continue
        for i, j in enumerate(newfloors['ts']):
            # Insert data where timestamp is same
            if j == val:
                p = re.compile(newfloors['traj'][i])
                if p.findall(dev_data['traj'][idx]) : # when same floor like (if and if-inner)
                    dev_data['ts'].insert(idx, newfloors['ts'][i])
                    dev_data['ts_end'].insert(idx, newfloors['ts_end'][i])
                    dev_data['traj'].insert(idx, newfloors['traj'][i])
                    dev_data['dwell_time'].insert(idx, newfloors['ts_end'][i] - newfloors['ts'][i])
                else : # when same ts but different floor name like (1f and in)
                    dev_data['ts'].insert(idx+1, newfloors['ts'][i])
                    dev_data['ts_end'].insert(idx+1, newfloors['ts_end'][i])
                    dev_data['traj'].insert(idx+1, newfloors['traj'][i])
                    dev_data['dwell_time'].insert(idx+1, newfloors['ts_end'][i] - newfloors['ts'][i])
                
                break
        val_last = val # Save present value to val_last for comparison

    return dev_data

############################
## Floor Data Saving Part ##
############################

def floorDataSaver(df, tuple_index, dev_data):
    # Save data
    dff = df.copy()
    dff.loc[dff.index[tuple_index], : ] = dev_data
    return dff


@timing.timing
def trajectoryPreprocessor(df):
    # count = 0 # version for df.iterrows.

    #for tuple_index, dev_data in df.iterrows(): # version for df.iterrows.
    for tuple_index in range(0, len(df)): # version for df.loc[df.index[tuple_index]]
        # print(count) # version for df.iterrows.
        # dev_data = df.loc[df.index[tuple_index], ['traj', 'ts', 'ts_end', 'dwell_time']] 
        dev_data_before = df.loc[df.index[tuple_index]] # version for df.loc[df.index[tuple_index]]
        date_device_id = dev_data_before['date_device_id']
        floors = floorFinder(dev_data_before)
        if not floors : # if there is no specific floor
            continue
        dev_data_after = floorEraser(dev_data_before, floors)
        newfloors = floorCreator(dev_data_after)
        dev_data_after = floorInsert(dev_data_after, newfloors)
        dev_data_after['date_device_id'] = date_device_id
        df = floorDataSaver(df, tuple_index, dev_data_after) # version for df.loc[df.index[tuple_index]]
        

        # df = floorDataSaver(df, count, dev_data, newfloors) # version for df.iterrows.
        #count = count + 1 # version for df.iterrows.
        if random.random() < 0.001:
        	print('Trajectory preprocessing...   Currently %dth moving pattern...'%(tuple_index))

    df['hour_start'] = df['ts'].apply(lambda x: [int(datetime.datetime.fromtimestamp(y).strftime('%H')) for y in x])
    df['time_start'] = df['ts'].apply(lambda x: [datetime.datetime.fromtimestamp(y).strftime('%H:%M:%S') for y in x])  # %Y-%m-%d
    df['hour_end'] = df['ts_end'].apply(lambda x: [int(datetime.datetime.fromtimestamp(y).strftime('%H')) for y in x])
    df['time_end'] = df['ts_end'].apply(lambda x: [datetime.datetime.fromtimestamp(y).strftime('%H:%M:%S') for y in x])  # %Y-%m-%d

    return df










''' 
존재하는 moving pattern의 device id가 현재 방문 포함 총 몇 번 방문하였는지 계산 (parameter: revisit interval)
'''

### date와 device_id로 date_device_id를 다시 분리
def add_visitcount(mpframe2):
	mpframe2['device_id'] = mpframe2['date_device_id'].apply(lambda x: x[6:])
	mpframe2['date'] = mpframe2['date_device_id'].apply(lambda x: int(x[:5]))

	mpframe2['cnt'] = 1
	mpframe2['new_visit_count'] = mpframe2[['device_id', 'cnt']].groupby('device_id').cumsum()
	mpframe2['revisit_intention'] = 0
	revisit_interval_thres = 90

	for ids in mpframe2['device_id'].unique():
	    dff = mpframe2.loc[mpframe2['device_id']==ids]   
	    a = 0
	    date = min(dff.date)
	    prev_idx = ''
	    for index, row in dff.iterrows():
	        if a+1 == row['new_visit_count']:
	            if date+revisit_interval_thres > row['date']:
	#                 print('regular revisit: {0} days interval'.format(row['date']-date))
	#                 print('previous index: ',prev_idx)
	                mpframe2.set_value(prev_idx, 'revisit_intention', 1)
	#             elif row['new_visit_count'] == 1:
	#                 print('regular revisit: {0} days interval'.format(row['date']-date))
	#             else:
	#                 print('Irregular revisit: {0} days interval'.format(row['date']-date))
	            prev_idx = index
	#             print(row,'\n')
	            a = row['new_visit_count']
	            date = row['date']
	            
	# cols = mpframe2.columns.tolist()
	# newcols = cols[:9]+cols[22:23]+cols[21:22]+cols[10:11]+cols[12:13]+cols[11:12]+cols[13:21]+cols[24:]
	# newcols = newcols[:18]+newcols[20:22]+newcols[19:20]+newcols[18:19]+newcols[22:]
	# mpframe2 = mpframe2[newcols]
	del mpframe2['cnt']
	return mpframe2.ix[:-1]





@timing.timing
def remove_frequent_visitors(trajs_combined, revisit_interval, frequent_limit):
	### 최근 세달 이내 revisit intention이 없는 moving pattern 제거 
	trajs_combined = trajs_combined.set_index('date_device_id')

	limit_date = max(trajs_combined.date) - revisit_interval
	recent3monthvisitors = trajs_combined.loc[(trajs_combined.date > limit_date) & (trajs_combined.revisit_intention == 0)].index
	trajs_excludelast3months = trajs_combined.drop(recent3monthvisitors)

	### 10번 초 온 사람들은 제거.
	visitcounts = trajs_excludelast3months.groupby(['device_id'])['new_visit_count'].max()
	freqvisitors = visitcounts.loc[visitcounts > frequent_limit ].keys()
	trajs_freqremoved = trajs_excludelast3months.loc[-trajs_excludelast3months.device_id.isin(freqvisitors.tolist())]

	### Revisit intention 비율을 50대 50으로 맞춤 (sampling)
	trajs_1 = trajs_freqremoved.loc[trajs_freqremoved['revisit_intention']==1]
	trajs_0 = trajs_freqremoved.loc[trajs_freqremoved['revisit_intention']==0]
	new_trajs_0 = trajs_0.iloc[np.random.permutation(len(trajs_0))][:trajs_1.shape[0]]  ## trajs_1의 크기에 맞게 trajs_0을 랜덤 샘플링.

	### 1:1 비율의 dataframe 만들기
	trajs_combined_balanced = pd.concat([trajs_1, new_trajs_0])
	trajs_combined_balanced = trajs_combined_balanced.sample(frac=1)

	return trajs_combined_balanced

# placeNum = str(786)
# reindexed_picklePath = "../data/"+placeNum+"/"+placeNum+"_mpframe_160923.p"
# remove_frequent_visitors(trajs_combined, 90, 10)


def finalprocessing(mpframe4):
    cols = mpframe4.columns.tolist()
    newcols = cols[:12] + cols[13:] + cols[12:13]
    df_learning = mpframe4[newcols]
    df_learning = df_learning.fillna(0)
    df_learning = df_learning.reindex(np.random.permutation(df_learning.index))
    return df_learning


def getuniqueareas(trajseries):
	aggregated_traj = list(itertools.chain.from_iterable(trajseries))
	uniqueareas = sorted(list(set(aggregated_traj)))
	return uniqueareas



if __name__ == '__main__':
    placeNum = str(786)
    oldpath = "../data/"+placeNum+"/"+placeNum+"_mpframe.p"
    statistical_picklePath = "../data/"+placeNum+"/"+placeNum+"_mpframe3.p"
    postprocessingPath = "../data/"+placeNum+"/"+placeNum+"_preprocessor_result_sample.p"

    dff = pd.read_pickle(oldpath)
    df_sample = dff.head(1000)
    df_sample_after = trajectoryPreprocessor(df_sample)
    df_sample_after.to_pickle(postprocessingPath)




