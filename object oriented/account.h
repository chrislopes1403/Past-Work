/*Chris Lopes
 COP3330 FALL 2018
 Section 1

*/

#include <iostream>
using namespace std;

class Accounts
{
public:
virtual double ProjectedBalance()=0;
virtual  const char* ReturnFirstName() const=0;
virtual  const char* ReturnLastName() const=0;
virtual  const char* ReturnType() const=0;
virtual double CurrentBalances()=0;


Accounts();
Accounts(char*,char*,char*);
protected:
char firstname[20];
char lastname[20];
char accounttype[20];
};

class Checkings: public Accounts
{
public: 
double ProjectedBalance();
double CurrentBalances();
Checkings();

Checkings(char*,char*,char*,double);

const char* ReturnFirstName() const;
const char* ReturnLastName() const;
const char* ReturnType() const;  
private:
double CurrentBalance;
double Interest;
};


class Investments: public Accounts
{
public: 
double ProjectedBalance();
double CurrentBalances();
Investments();
Investments(char*,char*,char*,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double,double);
const char* ReturnFirstName() const;  
const char* ReturnLastName() const;           
const char* ReturnType() const;    
private:
double ETF1[4];
double ETF2[4];
double ETF3[4];
double ETF4[4];
double ETF5[4];

};

class Savings: public Accounts
{
public: 
double ProjectedBalance();
Savings();
Savings(char*,char*,char*,double,double);
const char* ReturnFirstName() const;  
const char* ReturnLastName() const;           
const char* ReturnType() const;    
double CurrentBalances();
private:
double CurrentBalance;
double Interest;

};


