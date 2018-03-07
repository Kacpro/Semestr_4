import math
#import math as m
#from math import *

# w konsoli: dir(math) - lista funkcji w bibliotece math
# math.sqrt.__doc__ - dokumentacja funkcji

def main():
	a = float(input("a = "))
	b = float(input("b = "))
	c = float(input("c = "))

	delta = b*b - 4*a*c
	if delta > 0 : 
		x1 = (-b-math.sqrt(delta))/(4*a)
		x2 = (-b+math.sqrt(delta))/(4*a)
		print("x1=",x1, "x2=", x2)
	elif delta == 0:
		x = -b/(4*a)
	#	print("x=", x)
	#	print("x = %0.4f"%x) jak więcej zmiennych to w krotce
		print("x ={:0.4f}".format(x))
	else:
		print("Brak rozwiązań")
	print("Do widzenia")

#if __main__ == "main":
#	main()

main()