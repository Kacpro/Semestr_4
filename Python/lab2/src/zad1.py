def foo(s):
    a = []
    for letter in s:
        if (letter.isalpha()):
            a.append(letter);
    return sorted(set(a))


op = '&>|'

#var = 'abcdefghijklmnopqrstuvwxyz'

def check(exp):
    ln = 0
    state = True
    for z in exp:
        if z == '(': ln = ln + 1
        if z == ')': ln = ln - 1
        if ln < 0: return False
        if state == True:
            if z in foo(exp): state = False
            elif z in ')'+op: return False
        else:
            if z in op: state = True
            elif z in '('+ foo(exp) : return False
    if ln != 0: return False
    return not state             
            

def bal(w, op):
    ln = 0
    for i in range(len(w)-1, 0, -1):
        if w[i] == '(': ln += 1
        if w[i] == ')': ln -= 1
        if w[i] in op and ln == 0: return i
    return -1


def onp(w):
    while w[0] == '(' and w[-1] == ')' and check(w[1:-1]):
        w = w[1:-1]
    p = bal(w, '>')
    if p>=0:
        return onp(w[:p]) + onp(w[p+1:]) + w[p]
    p = bal(w, '&|')
    if p>=0:
        return onp(w[:p]) + onp(w[p+1:]) + w[p]
    return w


def mapuj(wyr, zm, val):
    l = list(wyr)
    for i in range(len(l)):
        if zm.count(wyr[i]) > 0: 
            p = zm.index(wyr[i])
            l[i] = val[p]
    return ''.join(l)


def value(wyr, val):
    zm = foo(wyr)  #var
    wyr = mapuj(wyr, zm, val)
    st = []
    for z in wyr:
        if z in '01': st.append(int(z))
        elif z == '&': st.append(st.pop() and st.pop())
        elif z == '|': st.append(st.pop() or st.pop())
        elif z == '>': st.append(st.pop() or 1-st.pop())
    return st.pop()


def gen(n):
    for i in range(2**n):
        yield bin(i)[2:].rjust(n,'0')


def evaluate(expr):
	for val in gen(len(foo(expr))):
    		print(val[:len(foo(expr))] + '  ' + str(value(onp(expr), val)))


def isTautology(expr):
    for val in gen(len(foo(expr))):
        if value(onp(expr), val) == 0: return False
    return True


def areTheSame(expr1, expr2):
    if foo(expr1) != foo(expr2): return False
    for val in gen(len(foo(expr1))):
        if value(onp(expr1), val) != value(onp(expr2), val): return False
    return True


evaluate('a&b|c')











