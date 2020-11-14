#include<iostream>
#include "list-CL.h"
#include <string>
using namespace std;

int main()
{
listclass<string> s1;
string x;
string y;
string z;
string w;
string u;
string h;
x="chris";
y="lopes";
z="bob";
w="hoe";
u="lil";
h="nigga";
s1.back_push(x);
s1.back_push(y);
s1.back_push(z);
s1.back_push(w);
s1.back_push(u);
s1.back_push(h);

listclass<string>::theiterator it;
it = s1.begin(); 
string hold;
string hold2;

for(int i=0; i<s1.getSize(); i++)
{
hold=*it++;
}


return 0;
}
