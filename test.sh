#! /bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" >tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 '12 + 34 - 5;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 27 '3*3*3;'
assert 0 '-1+1;'
assert 3 '-1+(+5-1);'
assert 1 '-1+(-5+7);'
assert 2 '(-5+7);'
assert 1 '1==1;'
assert 1 '1>=1;'
assert 0 '1!=1;'
assert 0 '1>1;'
assert 0 '2<1;'
assert 1 '1<2;'
assert 5 'a=5;'
assert 12 'a=10;a+2;'
assert 1 'a=6;a-5;'
assert 13 'a=6;b=7;a+b;'
assert 101 'a=100;b=1;a+b;'
assert 11 'foo=1;bar=10;foo+bar;'
assert 10 'foo=1;bar=2+7;foo+bar;'

echo ok
