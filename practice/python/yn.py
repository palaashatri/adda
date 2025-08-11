# name = ""
# while name != "alice":
# 	print("please type your name")
# 	name = input()
# print("thank u")


name = ""
while True:
	print("please type your name")
	name = str(input())
	if name == "alice" or name == "kjd":
		print("thank you")
		break

print("Done")