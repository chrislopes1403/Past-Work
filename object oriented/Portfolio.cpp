/*Chris Lopes
 COP3330 FALL 2018
 Section 1

*/



#include<iostream>
#include<string>
#include "Portfolio.h"
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include<cstring>
using namespace std;

//Constructors
//--------------------------------------------------
Portfolio::Portfolio()
{
current=0;
max=5;
accountlist= new Accounts*[max];
}

Portfolio::~Portfolio()
{
delete [] accountlist;
}
void Portfolio::Grow()
{
max=current+5;
Accounts**temp= new Accounts*[max];

    for(int i=0; i<current; i++)
        temp[i]=accountlist[i];

delete[]accountlist;
accountlist=temp;
}
//Accesor functions
//-------------------------------------------------------------------------------
void Portfolio::showAccounts() const
{
cout<<endl;
cout<<left << setw(30)<<"Holder"<<left << setw(15)<<"Type"<<left << setw(15)<<"Balance"<<endl;
cout<<endl;
    for(int i=0; i<current; i++)
    {
        cout<< left << setw(15)<< accountlist[i]->ReturnLastName() 
        <<left << setw(15)<< accountlist[i]->ReturnFirstName()
        <<left << setw(15)<<accountlist[i]->ReturnType()
        <<left << setw(15)<<fixed<<setprecision(2)<< accountlist[i]->CurrentBalances()<<endl;
    }
cout<<endl;
}
//Mutator Functions
//----------------------------------------------------------
bool Portfolio::importFile(const char *filename)
{
ifstream in1;

in1.open(filename);

    if(!in1)
    {
        return false;
    }
    else
    {
    in1>>current;
    in1.ignore();


        if(!(current2==0))
        {
            current=current+current2;
        }
        if(current>max)
            Grow();

        for(int i=current2;i<current;i++)
        {
        in1.getline(lastname,20,',');
        in1.ignore(1);
        in1.getline(firstname,20);
        in1.getline(type,20,' ');

            if(current>=max)
                Grow();


            if(strcmp(type,"Checking")==0)
            {
                //cout<<"find"<<endl;
                in1>>holder[0];
                accountlist[i]=new Checkings(firstname,lastname,type,holder[0]);                
            }

            if(strcmp(type,"Savings")==0) 
            {
                in1>>holder[0]>>holder[1];
                accountlist[i]=new Savings(firstname,lastname,type,holder[0],holder[1]);                       
            }

            if(strcmp(type,"Investment")==0)  
            {
                in1>>holder[0]>>holder[1]>>holder[2]>>holder[3]>>holder[4]>>holder[5]
                >>holder[6]>>holder[7]>>holder[8]>>holder[9]>>holder[10]>>holder[11]
                >>holder[12]>>holder[13]>>holder[14]>>holder[15]>>holder[16]>>holder[17]
                >>holder[18]>>holder[19];

                accountlist[i]=new Investments(firstname,lastname,type,holder[0],holder[1],holder[2],
                holder[3],holder[4],holder[5],holder[6],holder[7],holder[8],holder[9],holder[10],holder[11],
                holder[12],holder[13],holder[14],holder[15],holder[16],holder[17],
                holder[18],holder[19]);                    
            }
        in1.ignore();                //for loop
        }
    in1.close();
    current2=current;              
    return true;
    }
}

bool Portfolio::createReportFile(const char* filename)
{
double x=0,xtotal=0;
double y=0,ytotal=0;
double z=0,ztotal=0;

ofstream out1;
out1.open(filename);

    if(!out1)
        return false;
    else
    {
        out1<<"Banking Summary"<<endl;
        out1<<"--------------------------"<<endl;
        out1<<endl;
        out1<<"Savings Accounts"<<endl;
        out1<<endl;
        out1<<"Holder's Name                                 Initial Balance         Projected Balance"<<endl;
        out1<<"-------------------------------------------------------------------------------------------"<<endl;

            for(int i=0; i<current; i++)
            {
                if(strcmp(accountlist[i]->ReturnType(),"Savings")==0){
                    out1<< left << setw(12)<< accountlist[i]->ReturnLastName() << left << setw(35)<< accountlist[i]->ReturnFirstName()<<left << setw(25)
                    <<fixed<<setprecision(2)<< accountlist[i]->CurrentBalances()
                    <<fixed<<setprecision(2)<< accountlist[i]->ProjectedBalance()<<endl;
                    x++;
                    xtotal=xtotal+accountlist[i]->ProjectedBalance();
                }
            }
        out1<<endl;
        out1<<"Ceckings Accounts"<<endl;
        out1<<endl;
        out1<<"Holder's Name                                  Initial Balance        Projected Balance"<<endl;
        out1<<"-------------------------------------------------------------------------------------------"<<endl;

            for(int i=0; i<current; i++)
            {
                if(strcmp(accountlist[i]->ReturnType(),"Checking")==0){
                    out1<< left << setw(12)<< accountlist[i]->ReturnLastName() << left << setw(35)<< accountlist[i]->ReturnFirstName()<<left << setw(25)
                    <<fixed<<setprecision(2)<< accountlist[i]->CurrentBalances()
                    <<fixed<<setprecision(2)<< accountlist[i]->ProjectedBalance()<<endl;
                    y++;
                    ytotal=ytotal+accountlist[i]->ProjectedBalance(); 
                }
            }

        out1<<endl;
        out1<<"Investment Accounts"<<endl;
        out1<<endl;
        out1<<"Holder's Name                                  Initial Balance        Projected Balance"<<endl;
        out1<<"-------------------------------------------------------------------------------------------"<<endl;


            for(int i=0; i<current; i++)
            {
                if(strcmp(accountlist[i]->ReturnType(),"Investment")==0){
                    out1<< left << setw(12)<< accountlist[i]->ReturnLastName() << left << setw(35)<< accountlist[i]->ReturnFirstName()<<left << setw(25)
                    <<fixed<<setprecision(2)<< accountlist[i]->CurrentBalances()
                    <<fixed<<setprecision(2)<< accountlist[i]->ProjectedBalance()<<endl;
                    z++;
                    ztotal=ztotal+accountlist[i]->ProjectedBalance(); 
                }
            }

        out1<<endl;
        out1<<endl;
        out1<<"Overall Account Distrabution"<<endl;
        out1<<endl;

            if(x>0)
            {
                out1<<left<<setw(12)<<"Savings:"<<left<<setw(5)<<x<<left<<setw(5)<<'-'<<left<<setw(5)
                <<fixed<<setprecision(2)<<(xtotal/x)<<endl;
            }
            if(y>0)
            {
                out1<<left<<setw(12)<<"Checkings:"<<left<<setw(5)<<y<<left<<setw(5)<<'-'<<left<<setw(5)      
                <<fixed<<setprecision(2)<<(ytotal/y)<<endl;
            }
            if(z>0)
            {
                out1<<left<<setw(12)<<"Investments:"<<left<<setw(5)<<z<<left<<setw(5)<<'-'<<left<<setw(5)      
                <<fixed<<setprecision(2)<<(ztotal/z)<<endl;
            }

    }
out1.close();
return true;
}

