# Drlang
### :star: A Hobby Programming Lanuage Made With 'Dari' KeyWords using C

> \[!IMPORTANT]
>
> The Lanuage Is Still In Development

* #### :rocket: Synatx
    * let => mthwl
    * if  => aga
    * else => aga_ni
    * for => ta
    * while => twakht_ki
    * == => basha
    * != => nabasha
    * = => hast
    * < => khord_az
    * '>' => kalan_az
    * print => parto
    * '+' => jama
    * '-' => manfi
    * '/' => tqsim
    * '*' => zarb

### Examples
#### Factorial
```
mthwl res hast 1;

// n input hast
mthwl n hast 10;

ta(mthwl i hast 2;i khord_az n jama 1;i hast i jama 1){
   res hast res zarb i;
}

parto res;


```
#### Fibonacci Numbers
```
// fibonacci impl in drlang

mthwl a hast 0;
mthwl b hast 1;

// n is the input
mthwl n hast 10;


// 0 1 2 3 4 5 6  7  8  9 10
// 0 1 1 2 3 5 8 13 21 34 55


ta(mthwl i hast 2;i khord_az n jama 1;i hast i jama 1){
	mthwl temp hast b;
	b hast b jama a;
	a hast temp;
}

parto "The result is :";
parto b;

```
