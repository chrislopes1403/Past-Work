/*Chris Lopes
 COP3330 FALL 2018
 Section 1
*/


#include "account.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include<string>
using namespace std;


//Base class constructor
//=======================================================
Accounts::Accounts()
{
strcpy(firstname,"default");
strcpy(lastname,"default");
strcpy(accounttype,"default");
};

Accounts::Accounts(char*first,char*last,char*type)
{
strcpy(firstname,first);
strcpy(lastname,last);
strcpy(accounttype,type);

};
//Child class contructors
//---------------------------------------------------------------------
Checkings::Checkings() : Accounts(firstname,lastname,accounttype)
{
CurrentBalance=0;
}

Checkings::Checkings(char*first,char*last,char*type,double balance): Accounts(first,last,type)
{
CurrentBalance=balance;
}


Savings::Savings(): Accounts(firstname,lastname,accounttype)
{
Interest=0;
CurrentBalance=0;
}

Savings::Savings(char*first,char*last,char*type,double balance,double inv ) :  Accounts(first,last,type)
{
Interest=inv;
CurrentBalance=balance;
}

Investments::Investments() : Accounts(firstname,lastname,accounttype)
{
for(int i=0; i<4; i++)
ETF1[i]=0;
for(int i=0; i<4; i++)
ETF2[i]=0;
for(int i=0; i<4; i++)
ETF3[i]=0;
for(int i=0; i<4; i++)
ETF4[i]=0;
for(int i=0; i<4; i++)
ETF5[i]=0;
}

Investments::Investments(char*first,char*last,char*type,double t1,double t2,double t3,double t4,double t5,double t6,double t7,double t8,double t9,double t10,double t11,double t12,double t13,double t14,double t15,double t16,double t17,double t18,double t19,double t20):  Accounts(first,last,type)
{ 
ETF1[0]=t1;
ETF1[1]=t2;
ETF1[2]=t3;
ETF1[3]=t4;

ETF2[0]=t5;
ETF2[1]=t6;
ETF2[2]=t7;
ETF2[3]=t8;

ETF3[0]=t9;
ETF3[1]=t10;
ETF3[2]=t11;
ETF3[3]=t12;

ETF4[0]=t13;
ETF4[1]=t14;
ETF4[2]=t15;
ETF4[3]=t16;

ETF5[0]=t17;
ETF5[1]=t18;
ETF5[2]=t19;
ETF5[3]=t20;
}
//Mutator functions
//-------------------------------------------------
double Checkings::ProjectedBalance()
{
double total;

total=CurrentBalance+0.1;

return total;
}

double Savings::ProjectedBalance()
{                
double total;

total=CurrentBalance*(1+Interest);

return total;
}

double Investments::ProjectedBalance()
{                
double CurrentValue;
double DIV;
double total;
CurrentValue=((ETF1[0]/ETF1[1])*ETF1[2]) + ((ETF2[0]/ETF2[1])*ETF2[2]) + ((ETF3[0]/ETF3[1])*ETF3[2]) + ((ETF4[0]/ETF4[1])*ETF4[2]) + ((ETF5[0]/ETF5[1])*ETF5[2]);
DIV=(ETF1[3]*ETF1[0]) + (ETF2[3]*ETF2[0]) + (ETF3[3]*ETF3[0]) + (ETF4[3]*ETF4[0]) + (ETF5[3]*ETF5[0]);
total = DIV + CurrentValue;
return total;
}
//Accessor fuction
//-------------------------------------------------------
double Checkings::CurrentBalances(){
return CurrentBalance;}
double Savings::CurrentBalances(){
return CurrentBalance;}
double Investments::CurrentBalances()
{
double total;
total= (ETF1[0] +  ETF2[0] + ETF3[0] +  ETF4[0] + ETF5[0]);
return total;
}
//------------------------------------------------------










//------------------------------------------------------
const char* Checkings::ReturnFirstName() const{
return firstname;}  
const char* Checkings::ReturnLastName() const{
return lastname;}           
const char* Checkings::ReturnType() const{
return accounttype;}    
//-------------------------------------------------------

const char* Savings::ReturnFirstName() const{
return firstname;}
const char* Savings::ReturnLastName() const{
return lastname;}
const char* Savings::ReturnType() const{
return accounttype;}  
//--------------------------------------------------------------

const char* Investments::ReturnFirstName() const{
return firstname;}
const char* Investments::ReturnLastName() const{
return lastname;}
const char* Investments::ReturnType() const{
return accounttype;}  
//----------------------------------------------------------------

