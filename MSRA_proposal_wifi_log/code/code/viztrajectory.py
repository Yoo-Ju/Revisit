import pickle
import pandas as pd
import numpy as np
import plotly.plotly as py
from plotly.tools import FigureFactory as FF
import datetime

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
    qd['t_start'] = qd['Start'].apply(lambda x: datetime.datetime.fromtimestamp(x/1000).strftime('%H:%M:%S'))
    qd['t_end'] = qd['Finish'].apply(lambda x: datetime.datetime.fromtimestamp(x/1000).strftime('%H:%M:%S'))
    # .as_matrix()
    return qd

def printchartlah(i):
    print("---------------", i,'th longest moving pattern: Sequence #', largg[i])
    qd = ganttprint(mpframe, largg[i])  ## .sort_index(ascending=False)
    print(qd[['Task','t_start', 't_end']])
    return qd

if __name__ == '__main__':
	fn = "../../data/786/786_mpframe_160923.p"
	mpframe = pd.read_pickle(fn)
	lenlogs = np.asarray([len(x) for x in mpframe.logs])
	indxx = np.argsort(lenlogs, )
	largg = indxx[::-1]


	nth = 4000
	qd2 = printchartlah(nth)

	fig2 = FF.create_gantt(qd2[['Task','Start', 'Finish', 'Complete']], colors='Viridis', index_col='Complete', show_colorbar=True)
	# p2 = py.iplot(fig2, filename='Numeric Variable', world_readable=True)
	name = './plot/'+'trajectory_'+fn+'_'+nth
	py.image.save_as(fig2, filename=name, format='png', width='1800', height='1200')