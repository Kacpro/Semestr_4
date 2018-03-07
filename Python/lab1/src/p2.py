import sump as s

# liczby doskonałe
for i in range(1,1000):
	if s.sump(i) == i:
		print(i)

print("----------")

#liczby zaprzyjaźnione
for i in range(1,10000):
	if s.sump(s.sump(i)) == i:
		print(i, s.sump(i))