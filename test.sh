#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but $actual"
		exit 1

	fi

}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 3 '1; 2; 3;'

assert 0 'm=a=0;'
assert 42 'k=42;'
assert 21 'e=5+20-4;'
assert 41 'g= 12 + 34 - 5 ;'
assert 47 'c=5+6*7;'
assert 15 't=5*(9-6);'
assert 4 's=(3+5)/2;'
assert 10 'd=-10+20;'
assert 10 'd=- -10;'
assert 10 'a=- - +10;'

assert 0 'v=0==1;'
assert 1 'c=42==42;'
assert 1 'a=0!=1;'
assert 0 'x=42!=42;'

assert 1 'q=0<1;'
assert 0 'e=1<1;'
assert 0 'h=2<1;'
assert 1 'z=0<=1;'
assert 1 'f=1<=1;'
assert 0 'f=2<=1;'

assert 1 'z=1>0;'
assert 0 'c=1>1;'
assert 0 'b=1>2;'
assert 1 'a=0>=0;'
assert 0 'a=0>=1;'
assert 0 'a=0>=2;'

assert 3 'abs=3;'
assert 8  'ab=2*4;'
assert 7  'li=3+4;'
#assert -1  'y=1-2;'     //unsign logic
assert 7  'val=8;asfjk=10;val=9;val=7;'


echo OK
