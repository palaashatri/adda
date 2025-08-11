# take input from user for name. If name is equals to "Alice", print Hi Alice. In the end print Done.

print("what is your name")

name = str(input())
if name == 'Alice' or name == 'alice' :
	print("hi Alice")

# take input for age. if age <18, print you're a child
print("What is your age?")
age = int(input())
if age < 18:
	print('you\'re a child')
elif 18 <= age <= 40 :
	print("hello there")
else:
	print("roger roger")

print("done")