/*
CHRIS LOPES
COP 4530
SUMMER 2019
HW2 
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <cstring>
#include <list>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <unordered_map>
using namespace std;

int main(int argc, char* argv[])
{
string select;
string x;
string y;
char z;
int w=1;
int u=0;
int g=0;
int num=0;
int num2=0;
ifstream file;

num=atoi(argv[2]);
num2=atoi(argv[3]);

unordered_map<string,list<string>> thekey;

file.open(argv[1]);
        while(!(file.eof()))
	{
        z=file.peek();
                while(isspace(z))
                {
                        file.ignore(1);
                        z=file.peek();
                }
	
        getline(file,y,'	');
        getline(file,x);     
	
		string temp;
                string full;
                full=x;
                temp=x;   
		if(thekey.find(x) == thekey.end())
  		{
			for(int i=1; i<=x.size(); i++)
			{
			temp.resize(i);
			 thekey[temp].push_back(full);
                	 thekey[temp].push_back(y);
			temp=full;
  			}
		}
		else{
		thekey[x].push_back(full);
		thekey[x].push_back(y);	
		}	
	}
	
cout<<"Ask me which is most popular!"<<endl;
cin>>select;

do
{
    if(thekey.find(select)==thekey.end())
        {
	cout<< "key not found"<< endl;
        }
        else
        {        
	cout<<"Top answers for '"<<select<<"'"<<endl;
	list <string>::iterator output= thekey[select].begin();
		for(output=thekey[select].begin(); output!=thekey[select].end(); output++)
		{
		int stringsize;
		string hold1;
		string hold2;
		hold1=*output;
		hold2=*++output;
		stringsize=hold1.size();
		if((u<num)&&stringsize<=num2)
                {
                        cout<<"\t" <<w<<" "<<hold1<<" ("<<hold2<<" instances)"<<endl;
                        u++;
                        w++;
                }   
		
		          
		}
w=1;
u=0;
	}
cout<<"Ask me for another autocomplete"<<endl;
cin>>select;  
}while(select!="?");


file.close();
return 0;
}

