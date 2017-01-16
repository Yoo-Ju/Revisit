'''
Name: crawler.py
Date: 2016-08-23
Description: shops/781(커스텀멜로우 홍대점), shops/786(코오롱 스포츠 문정직영점)에 대해 크롤링함
'''
__author__ = 'Sundong Kim: sundong.kim@kaist.ac.kr'

import urllib.request, json, re
import pandas as pd
import datetime
import time, calendar
import glob
import os
import sys

def crawling(*args):  ### arguments example: 781, 786
    for placeNum in args:
        # placeNum = str(786)
        user_token = '77eFJPB37UkgYyLjWZrx'
        user_email = 'jeeyeachung@kolon.com'

        urlstrip1 = "http://walkinsights.com/api/v1/shops/"+placeNum+"/sessions?"
        urlstrip2 ="limit=1000&user_token="+user_token+"&user_email="+user_email
        url = urlstrip1 + urlstrip2
        response = urllib.request.urlopen(url)
        str_response = response.read().decode('utf-8')
        obj = json.loads(str_response)
        ts = obj['since']


        if not os.path.exists('../data2/'+placeNum):
            os.makedirs('../data2/'+placeNum)

        with open('../data2/'+placeNum+'/'+placeNum+'_'+ts+'.json', 'w') as f:
            json.dump(obj, f)
        f.close()

        while(True):
            urlstrip3 = "since="+ts+"&limit=1000&"
            url = urlstrip1 + urlstrip3 + urlstrip2
            response = urllib.request.urlopen(url)
            str_response = response.read().decode('utf-8')
            obj2 = json.loads(str_response)
            filename = '../data2/'+placeNum+'/'+placeNum+'_'+ts+'.json'
            with open(filename, 'w') as f:
                json.dump(obj2, f)
            f.close()
            try:
                ts = obj2['since']
            except KeyError:
                print("Crawling is done")   
                break


        contents = []
        files = glob.glob('../data2/'+placeNum+'/*.json')
        for filename in files:
            with open(filename, 'r') as f:
                d = json.load(f)
                contents.extend(d['sessions'])

        df = pd.DataFrame(contents)
            
        device_ids = df['device_id'].unique()
        

        df.to_pickle('../data2/'+placeNum+'/'+placeNum+'.p')
        print('Data for shop number %s has been crawled' % placeNum)
        print ('The number of detected unique device ids is %s' % len(device_ids))
        print ('The number of total logs is ', df.shape)
        print('Saved in %s is done' % '../data2/'+placeNum+'/'+placeNum+'.p')


if __name__ == '__main__':
    crawling(sys.argv[1], sys.argv[2])


