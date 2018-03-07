def sump(n):
	sum = 1
	p = 2
	while p*p <= n:
		if n%p == 0: sum = sum + p + n//p
		p += 1
	if p*p == n:
		sum += p
	return sum
	
	
def sump2(n):
	sum = 0
	p = 1
	while p < n:
		if n%p == 0: sum = sum + p
		p += 1
	return sum

	
if __name__ == "__main__" : sump(120)
# kod nie wykonuje się przy każdym imporcie