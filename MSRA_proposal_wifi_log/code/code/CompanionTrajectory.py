import os
import pandas
import collections
import re

########################
## Floor Finding Part ##
########################


def specificFloorFinder(dev_data, num_of_s_floor):
    # Extracts and returns specific floors and indices and True value in a form of list
    # only if there are at least 3 specific floors else False
    # Third element True is to distinguish whether used 
    # or not in iteration of deviceComparator
    s_floors = []
    indexer = []
    p = re.compile("^.*-")
    
    for idx, trajs in enumerate(dev_data['traj']):
        # if data is floor-specific data
        if p.findall(trajs):
            # get indexes where specific floor is.
            indexer.append(idx)
            s_floors.append(trajs)
    
    # vals = [dev_data['traj'][i] for i in indexer]
    #print(dev_data['traj'])
    #print(indexer)
        #[10, 19, 22, 23, 27, 33, 38, 39]
    #print(s_floors)
        #['2f-left', '1f-right', '1f-right', '1f-inner', '1f-right', '1f-right', '2f-inner', '2f-left']
    #print(vals)
        #['2f-left', '1f-right', '1f-right', '1f-inner', '1f-right', '1f-right', '2f-inner', '2f-left']
        
    # returns specific floors and indices only if there are at least 3 specific floors else False
    # Third element True is to distinguish whether used or not in iteration of deviceComparator
    return [s_floors, indexer] if len(s_floors)>=num_of_s_floor else False

############################
## Device Extracting Part ##
############################

def deviceExtractor(df_sorted_by_time, num_of_s_floor):
    # Create devices for companion checking using OrderedDict for easier implementation in the future.
    # Bring all trajectory data and extract trajectory tuples which has more than 3 specific floors
    # and return devices after saving data
    devices = collections.OrderedDict()
    
    # Iter through tuples 
    for idx, device_index in enumerate(df_sorted_by_time.index):

        # df.index is sorted by ts and enumerates according to sorted result.
        # idx for # th of tuple and device_index for original index of that tuple
        dev_data = df_sorted_by_time.loc[device_index, ['traj']]
        
        specificFloors = specificFloorFinder(dev_data, num_of_s_floor)
        # If there are at least 3 specific floors
        if(specificFloors):
            # then put specific floors, indices of them and tuple's index inside dictionary
            # if specificFloors is False, trajectory tuple won't be saved
            # because there is less than 3 specific floors.
            devices[df_sorted_by_time['date_device_id'][device_index]] = specificFloors
            devices[df_sorted_by_time['date_device_id'][device_index]].append(device_index)
    
    return devices


############################
## Device Comparing Part ###
############################

def deviceComparator(df_sorted_by_time, devices, num_of_s_floor):
    
    p = re.compile("^.*_")
    # Iterate devices' items to compare devices
    tuples = list(devices.items())
    
    for present_tuple_idx, tuple_value in enumerate(tuples):
        
        # Iterate from next one of present tuple until that day's last tuple
        for next_tuple_idx in range(present_tuple_idx+1, len(tuples)):
            
            # Compare days between present tuple's day and next tuple's day.
            # If days are not identical, then we go out of loop
            # because difference in days means no companions 
            if p.findall(tuples[present_tuple_idx][0])[0][:-1] != p.findall(tuples[next_tuple_idx][0])[0][:-1]:
                break
            
            # Check whether it is identified as two tuples are companion
            if companionChecker(df_sorted_by_time.ix[tuples[present_tuple_idx][1][2]], 
                            df_sorted_by_time.ix[tuples[next_tuple_idx][1][2]],
                            tuple_value[1], 
                            tuples[next_tuple_idx][1],
                            num_of_s_floor) :
                # Append date_device_id to each tuple when they are companion
                df_sorted_by_time.ix[tuples[present_tuple_idx][1][2]]['companion'].append(tuples[next_tuple_idx][0])
                df_sorted_by_time.ix[tuples[next_tuple_idx][1][2]]['companion'].append(tuples[present_tuple_idx][0])
                
    
    return df_sorted_by_time
            
#############################
## Companion Checking Part ##
#############################
    
def companionChecker(pt, nt, pt_s, nt_s, num_of_s_floor):    
    # pt = Present tuple from df
    # tt = Next tuple from df
    # pt_s = Present tuple of 'devices' excluding device id
    # nt_s = Next tuple of 'devices' excluding device id
    # companion Checker finds out accompanied trajectories and
    # return boolean value to show whether two tuples are companion or not
    
    
    # start_idx to save time by not iterating useless parts.
    start_idx = 0
    # number of trajectories accompanied
    accompanied_count = 0
    # Iterate through next tuple's specific floors and find out accompanied moments
    for nt_idx, nt_idx_val in enumerate(nt_s[1]):
        
        for pt_idx in range(start_idx, len(pt_s[1])):
            # When next tuple's traj ts time is later than ts_end time of present tuple
            # then save present index value + 1 for starting
            # and go to next iteration of inner loop
            if pt['ts_end'][pt_s[1][pt_idx]] < nt['ts'][nt_idx_val]:
                start_idx = pt_idx + 1 
                continue
            
            # When next tuple's traj ts_end time is earlier than ts time of present tuple
            # then get out of the loop and start next iteration of outer loop
            if pt['ts'][pt_s[1][pt_idx]] > nt['ts_end'][nt_idx_val]:
                break
            
            # From now there is time overlapping between two tuple traj.
            # Then we can check the names of specific floors
            # and add to accompanied_list
            if pt['traj'][pt_s[1][pt_idx]] == nt['traj'][nt_idx_val]:
                accompanied_count = accompanied_count + 1      
                break
        
        
        if accompanied_count >= num_of_s_floor:
            break   
                
    # Return boolean value to show whether two tuples are companion or not
    return True if accompanied_count >= num_of_s_floor else False

def companionFinder(df, num_of_s_floor):
    print("start")
    df_sorted_by_time = df.sort_values(['ts'], ascending=True) # Sort tuples according to start time of first timestamp
    df_sorted_by_time['companion']= df_sorted_by_time.apply(lambda x: [], axis=1)
    devices = deviceExtractor(df_sorted_by_time, num_of_s_floor)
    print("deviceExtractor finished")
    df_companion = deviceComparator(df_sorted_by_time, devices, num_of_s_floor)
    print("deviceComparator finished")
    df_companion['companion_count'] = df_companion['companion'].apply(lambda x: len(x))
    return df_companion
    
    
if __name__ == '__main__':
    df = pandas.read_pickle("../data/786_mpframe_trajprocessed.p")
    df_companion = companionFinder(df.head(500), 3)
    df_companion.to_pickle("../data/companion_result.p")

