'''
Name: predict.py
Date: 2016-10-13
Description: machine learning modules

Input: 
TO DO: 
'''

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.datasets import make_moons, make_circles, make_classification
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.discriminant_analysis import LinearDiscriminantAnalysis
from sklearn.discriminant_analysis import QuadraticDiscriminantAnalysis
from sklearn.linear_model import LogisticRegression
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.model_selection import cross_val_score
from sklearn import tree


def basicDecisionTree(X, y):
	clf = DecisionTreeClassifier(max_depth=5)
	return cross_val_score(clf, X, y, cv=10)   ## accuracy



# def experiments_100():
# 	names = ["3 Nearest Neighbors",  "Decision Tree",
# 	         "Random Forest", "AdaBoost",
# 	         "Naive Bayes", "Logistic Regression"
# 	        ]  ## "Linear SVM", "RBF SVM", "Linear Discriminant Analysis", "Quadratic Discriminant Analysis"
# 	classifiers = [
# 	    KNeighborsClassifier(3),
# 	#     SVC(kernel="linear", C=0.025),  ## very slow (can't wait)  
# 	#     SVC(gamma=2, C=1), ## very slow (can't wait)
# 	    DecisionTreeClassifier(max_depth=5),
# 	    RandomForestClassifier(max_depth=5, n_estimators=10, max_features=1),
# 	    AdaBoostClassifier(),
# 	    GaussianNB(),
# 	#     LinearDiscriminantAnalysis(),
# 	    LogisticRegression(penalty='l2', class_weight='balanced', solver='liblinear')
#     	]


# 	bigram_vectorizer = CountVectorizer(min_df=1, token_pattern='\w+\-*\w+', ngram_range=(1, 2))
# 	results_dict = dict()

# 	for i in range(100):
# 	    trajs_combined_balanced = balancing(trajs_combined)
# 	    df_learning = trajs_combined_balanced
# 	    df_learning = df_learning.fillna(0)
# 	    df_learning = df_learning.reindex(np.random.permutation(df_learning.index))
# 	    df_learning.tail(5)
# 	    data = np.asarray(df_learning)
# 	    X_small, y = data[:, 3:-1], data[:, -1].astype(int)

# 	    corpus = trajs_combined_balanced['traj']
# 	    corpvec2 = bigram_vectorizer.fit_transform(corpus)

# 	    X = np.concatenate((X_small, corpvec2.toarray()), axis=1)
# 	    newcolumns = df_learning.columns.tolist()[:-1]+bigram_vectorizer.get_feature_names()

# 	    scoring = ['accuracy', 'precision', 'recall', 'f1', 'roc_auc']
	    
	    
# 	    for name, clf in zip(names, classifiers):
# 	        results_dict.setdefault(name, dict())
# 	        for criteria in scoring:
# 	            score = cross_val_score(clf, X, y, cv=10, scoring=criteria)
# 	            avgscore = score.sum()/len(score)
# 	#             print('  *', name, criteria, round(avgscore,3))
# 	            results_dict[name].setdefault(criteria, []).append(avgscore)
