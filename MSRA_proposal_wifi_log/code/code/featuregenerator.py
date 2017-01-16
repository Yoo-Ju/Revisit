'''
Name: featuregenerator.py
Date: 2016-10-05
Description: 새로 indexed된 data frame을 기반으로 다양한 feature들을 만드는 모듈


Input: 
TO DO: 
'''
__author__ = 'Sundong Kim: sundong.kim@kaist.ac.kr'

import preprocessing
import timing
import pandas as pd
import datetime
import numpy as np
import re
import plotly.plotly as py
from plotly.tools import FigureFactory as FF

# placeNum = str(786)
# rawdata_picklePath = "../data/"+placeNum+"/"+placeNum+".p"
# reindexed_picklePath = "../data/"+placeNum+"/"+placeNum+"_mpframe_160923.p"

# df = pd.read_pickle(rawdata_picklePath)
# mpframe = pd.read_pickle(reindexed_picklePath)
# mpframe2 = add_stayingtime_eacharea(df, mpframe)


### input: n개의 row인데 각각의 row는 두개의 길이가 같은 list - 이 두 리스트를 받아서 각 area별로 머무른 시간 재정렬
def areaStayingAmountFeature(traj, time, areatypes):
    b = np.zeros(len(areatypes))
    for i in range(len(areatypes)):
        for j in range(len(traj)):
            if(areatypes[i] == traj[j]):
                b[i] += time[j]
    v = np.asarray([traj.count(area) for area in areatypes])
    return b

### 그냥 각 구역별로 얼마나 오래 커넥트됐는지 feature로 나타낸 후 dataframe에 추가  (NOT USED - 161130)
def add_stayingtime_eacharea(df, mpframe):
	areatypes = df.area.unique().tolist()

	base = np.zeros(len(areatypes))
	for i in range(mpframe.shape[0]):
	    arr = areaStayingAmountFeature(mpframe['traj'].ix[i],mpframe['dwell_time'].ix[i], areatypes)
	    base = np.concatenate((base, arr), axis=0)

	base = base.reshape((mpframe.shape[0]+1, 11))[1:,:]
	extft = pd.DataFrame(base, columns=areatypes)
	mpframe2 = pd.concat((mpframe, extft), axis=1)
	return mpframe2



def getIntervalUnion(a):
    b = []
    for begin,end in sorted(a):
        if b and b[-1][1] >= begin:
            b[-1] = (b[-1][0], end)
        else:
            b.append((begin, end))
    return b


def getSum(a):
    b = 0
    for interval in a:
        b = b + interval[1]-interval[0]
    return round(b)


def tryindex(x, cands):
    for i in cands:
        print(x.index(i))
    return 0
#     except ValueError:
#         return 0



def getRealDwellTime(lst, cands):
    idcs = [i for i, x in enumerate(lst[0]) if x in cands]
    intervals = [(lst[1][index], lst[2][index]) for index in idcs]
    return getSum(getIntervalUnion(intervals))



def division(a, b):
    try:
        value = a/b
    except ZeroDivisionError:
        value = float('Inf') 
    return value



def timestamp_to_day(x):
    a = x / 86400
    switcher = {
        0: [0, 0, 0, 1, 0, 0, 0], # "Thu",
        1: [0, 0, 0, 0, 1, 0, 0], # "Fri",
        2: [0, 0, 0, 0, 0, 1, 0], # "Sat",
        3: [0, 0, 0, 0, 0, 0, 1], # "Sun",
        4: [1, 0, 0, 0, 0, 0, 0], # "Mon",
        5: [0, 1, 0, 0, 0, 0, 0], # "Tue",
        6: [0, 0, 1, 0, 0, 0, 0]  # "Wed"
    }
    return switcher.get(int(a))


def statistical_feature_generator(lst, areaType):
    fs = []
    
    ### 총 로그 개수(f1)
    fs.append(len(lst['traj']))
    
    ### 각 구역별 머무른 시간(f2-f12)   
    for area in areaType:
        idcs1 = [i for i, x in enumerate(lst['traj']) if x == area]
        intervals = [lst['dwell_time'][index] for index in idcs1]
        fs.append(sum(intervals))  
    
    ### 실제 세부 구역 중 100초 이상 머무른 stay point들에서 머무른 시간의 평균, 표준편차 (f13-f14)
    ### 세부 구역에 머무른 로그 개수 중 stay point일 확률. (f15)
    idcs2_1 = [i for i, x in enumerate(lst['traj']) if x in ['1f-right', '1f-inner', '1f-left', '2f-right', '2f-inner', '2f-left', '3f']]
    idcs2_2 = [i for i, x in enumerate(lst['dwell_time']) if x > 100]
    idcs2_3 = list(set(idcs2_1) & set(idcs2_2))
    dtime = [lst['dwell_time'][index] for index in idcs2_3]
    value = division(len(idcs2_3), len(idcs2_2))
      
    fs.append(np.mean(dtime))
    fs.append(np.std(dtime))
    fs.append(value)
    
    
    ### Get Real time for 1f, 2f, 1f+2f+3f (f16-f18)
    cands1 = ['1f-right', '1f-inner', '1f-left']
    cands2 = ['2f-right', '2f-inner', '2f-left']
    cands3 = ['1f-right', '1f-inner', '1f-left', '2f-right', '2f-inner', '2f-left', '3f']
    realtime_1f = getRealDwellTime(lst, cands1)
    realtime_2f = getRealDwellTime(lst, cands2)
    realtime_total = getRealDwellTime(lst, cands3)
    fs.append(realtime_1f)
    fs.append(realtime_2f)
    fs.append(realtime_total)
    
    
    ### Quality measure we've discussed (f19-f21)
    fs.append(division(realtime_1f,fs[1]))
    fs.append(division(realtime_2f,fs[5]))
    fs.append(division(realtime_total,fs[10]))
    
    
    ### Day - categorical var (f22-f28) 
    remainder = lst['ts'][0]%604800
    day = timestamp_to_day(remainder)
    fs.extend(day)
    
    return fs


def add_statistical_features(df, df2):
    print('before', df2.shape)
    areaType = sorted(df.area.unique())
    features = df2.apply(lambda x: statistical_feature_generator(x, areaType), axis=1) 

    # df3 = pd.DataFrame(index = features.index)
    # print(df3.head(3))

    for i in range(len(features.iloc[0])):
        df2[i] = features.apply(lambda x: x[i])

    # df4 = pd.concat([df2, df3], axis=1, join='inner')
    return df2





'''
Indoor temporal movement features
'''

def puttogether(a, b):
    if b == 0:
        c = 'zero'
    elif b < 10:
        c = 'veryshort'
    elif b < 100:
        c = 'short'
    elif b < 1000:
        c = 'medium'
    else:
        c = 'long'
    return '-'.join([a,c])
    

def add_temporal_sign(x, area):  # Make features like 1f-medium, or 1f-inner-short
    idcs = [i for i, y in enumerate(x['traj']) if y in area]
    zt = [puttogether(x['traj'][index], x['dwell_time'][index]) for index in idcs] 
    return zt

def change_to_features(x, iunion):
    sss = 1001
    ddd = 1001
    for i in iunion:
        if str(x) == i:
            ddd = sss
        sss += 1
    return ddd


@timing.timing
def add_indoor_temporal_movement_features(z):
    area = preprocessing.getuniqueareas(z.traj)
    z1 = z.loc[z.revisit_intention == 1]
    z0 = z.loc[z.revisit_intention == 0]

    ### revisit intention 여부에 따라, top 25개 moving pattern feature를 뽑음
    ### add_temporal_sign: 1f-long 이런식으로 area와 머무른 시간을 붙여줌.
    iix1 = z1.apply(lambda x: add_temporal_sign(x, area), axis=1)
    iix0 = z0.apply(lambda x: add_temporal_sign(x, area), axis=1)
    i1 = iix1.apply(lambda x: str(x)).value_counts().sort_values(ascending=False).head(25).index
    i2 = iix0.apply(lambda x: str(x)).value_counts().sort_values(ascending=False).head(25).index
    
    ### iunion = 재방문율=1, 0일때 각각 25개씩 뽑은 후 그것들의 합집합.
    iunion = i1.union(i2)
    

    iix = z.apply(lambda x: add_temporal_sign(x, area), axis=1)

    z['ptn'] = iix.apply(lambda x: change_to_features(x, iunion))
    one_hot = pd.get_dummies(z['ptn'])
    del z['ptn']
    zdf = pd.concat([z, one_hot], axis=1, join='inner')
    return zdf












