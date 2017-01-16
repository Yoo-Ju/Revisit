'''
Name: reindex.py
Date: 2016-10-04
Description: 새로 moving patterns indexing


Input: 786.p - Output: 786_mpframe_160923.p
TO DO: try-catch statement according to input data availability
'''
__author__ = 'Sundong Kim: sundong.kim@kaist.ac.kr'

import timing
import pandas as pd
import datetime
import numpy as np
import re
# import plotly.plotly as py
# from plotly.tools import FigureFactory as FF


def finddata(logs, df, data):   # x is a list of logs
   return df.ix[logs][data].tolist()

def calend(x, y):
    return list(map(lambda i, j: i+j, x, y ))

def printall(mp, i, columnss):
    for column in columnss:
        print(mp[column].ix[i])


@timing.timing
def reindex_by_moving_pattern(df, mintime):
	### read data and reindex by date and device_id
	df.loc[:, 'date'] = df['ts'] // 86400000
	# df = df.loc[(df['dwell_time'] > 0)]
	df.loc[:, 'date_device_id'] = df.date.map(str) + "_" + df.device_id

	### collect valid trajectories (ex. trajecties containing only 'out' area, such as 'out-out-out-out' are deleted)
	traj = df.groupby(['date_device_id'])['area']
	ar = ['out']
	temp = traj.unique()
	temp2 = temp[-temp.apply(lambda x: (x == ar).all())]
	print('Ratio of valid trajectory having not only \'out\' area: ', len(temp2)/len(temp))
	df2 = df[df.date_device_id.isin(temp2.index.tolist())]


	### erase other useless trajectories having 'in' shorter than 100 seconds
	dfin = df2[df2['area'] == 'in']
	time = dfin.groupby(['date_device_id'])['dwell_time'].sum()
	time2 = time[time >= mintime]
	print('Ratio of trajectory staying \'in\' longer than 0 seconds: ', len(time2)/len(time))
	df3 = df2[df2.date_device_id.isin(time2.index.tolist())]

	del df, df2

	### make a list of logs for corresponding each indoor moving trajectories 
	mplogs = df3.sort_index(ascending=False).groupby('date_device_id', sort=False).apply(lambda x: x.index.tolist()).sort_index()
	mpframe = mplogs.to_frame(name='logs').reset_index()


	mpframe.loc[:, 'traj'] = mpframe['logs'].apply(lambda x: finddata(x, df3, 'area'))
	mpframe.loc[:, 'ts'] = mpframe['logs'].apply(lambda x: finddata(x, df3, 'ts'))
	mpframe.loc[:, 'dwell_time'] = mpframe['logs'].apply(lambda x: finddata(x, df3, 'dwell_time'))
	mpframe.loc[:, 'ts'] = mpframe['ts'].apply(lambda x: [int(y/1000) for y in x])
	mpframe.loc[:, 'hour_start'] = mpframe['ts'].apply(lambda x: [int(datetime.datetime.fromtimestamp(y).strftime('%H')) for y in x])
	mpframe.loc[:, 'time_start'] = mpframe['ts'].apply(lambda x: [datetime.datetime.fromtimestamp(y).strftime('%H:%M:%S') for y in x])  # %Y-%m-%d 
	mpframe.loc[:, 'ts_end'] = mpframe[['ts', 'dwell_time']].apply(lambda x: calend(*x), axis=1)
	mpframe.loc[:, 'hour_end'] = mpframe['ts_end'].apply(lambda x: [int(datetime.datetime.fromtimestamp(y).strftime('%H')) for y in x])
	mpframe.loc[:, 'time_end'] = mpframe['ts_end'].apply(lambda x: [datetime.datetime.fromtimestamp(y).strftime('%H:%M:%S') for y in x])

	del df3
	return mpframe




