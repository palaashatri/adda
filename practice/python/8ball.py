import random

def getanswer(answerno):
	if answerno == 1:
		return "maybe"
	elif answerno == 2:
		return "probably"
	elif answerno == 3:
		return "yes"
	elif answerno == 4:
		return "not sure, try again lol"
	elif answerno == 5:
		return "maybe later lmao"
	elif answerno == 6:
		return "concentrate and sybau"
	elif answerno == 7:
		return "hmm, no."
	elif answerno == 8:
		return "maybe not"
	elif answerno == 9:
		return "probably not"

print ("think of a yes/no question and press enter to see the result")
str(input())

print (getanswer(random.randint(1,9)))
