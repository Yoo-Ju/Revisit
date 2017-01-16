'''
Name: plotting.py
Date: 2016-10-04
Description: 새로 indexed된 data frame 중 특정 moving pattern을 plotly로 출력 


Input: 
TO DO: 
'''
__author__ = 'Sundong Kim: sundong.kim@kaist.ac.kr'


import pandas as pd
import datetime
import numpy as np
import re
import plotly.plotly as py
from plotly.tools import FigureFactory as FF


placeNum = str(786)
reindexed_picklePath = "../data/"+placeNum+"/"+placeNum+"_mpframe_160923.p"
mpframe = pd.read_pickle(reindexed_picklePath)

def ganttprint(mfs, i):
    
    qa = mfs['traj'].ix[i] + [1000*y for y in mfs['ts'].ix[i]] + [1000*z for z in mfs['ts_end'].ix[i]]
    l = int(len(qa)/3)
    qb = np.asarray(qa)
    shape = ( 3,l )
    qc = qb.reshape( shape ).T
    qd = pd.DataFrame(qc, columns=['Task', 'Start', 'Finish'])
    qd['Start'] = qd['Start'].astype('int') 
    qd['Finish'] = qd['Finish'].astype('int')
    qd['Complete'] = np.arange(40, 60, (60-40)/l)
    qd
    # .as_matrix()

    return qd

lenlogs = np.asarray([len(x) for x in mpframe.logs])
indxx = np.argsort(lenlogs, )
largg = indxx[::-1][:100]

qd = ganttprint(mpframe, largg[85])  ## .sort_index(ascending=False)
print(qd)

fig = FF.create_gantt(qd, colors='Viridis', index_col='Complete', show_colorbar=True)
py.iplot(fig, filename='Numeric Variable', world_readable=True)