/*Chris Lopes
 COP3330 FALL 2018
 Section 1
*/

#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include "account.h"
class Portfolio 
{
public:
Portfolio();         // starts out with empty accounts list
~Portfolio();        // cleanup (destructor)
bool importFile(const char *filename);
bool createReportFile(const char *filename);
void showAccounts() const;    // print basic list data
void Grow();

private:
Accounts**accountlist;
int max;
int current;
int current2;
double holder[20];
char firstname[20];
char lastname[20];
char type[20];
};
#endif //PORTFOLIO_H
