#!/usr/bin/python

from sklearn.externals import joblib
import random

def CalcScore(a,b,c,d,e,f,g,h):
	reg = joblib.load('reg.pkl')
	pic_features = [[a,b,c,d,e,f,g,h]]
	scoreone = reg.predict(pic_features)
	#score = int(random.randint(-3,2) + scoreone[0]*10 )
	score = int(b*2 + scoreone[0]*10 )
	SCORE = str(score)
	print ("Predicted value: ", SCORE)
	return SCORE 

if __name__ == '__main__':
	score = CalcScore(0.8, 0.8, 0.8, 0.8 , 0.8, 0.8 , 0.8, 0.8)
	print (score)
