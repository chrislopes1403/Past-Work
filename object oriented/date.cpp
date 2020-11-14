#include <iostream>
#include <iomanip>
#include "date.h"
using namespace std;

Date::Date()                                                                // default constructor    
{
	month=1;
	day=1;
	year=2000;
	format='D';
}

Date::Date(int m, int d, int y)                                       // constuctor that checks for valid parameters
{
	if(m>0&&m<13)
       {

		if(m==1 || m==3 || m==5 || m==7 || m==8 || m==10 || m==12)
        	{
      
  			if(d>0&&d<32)
                	{
                  		if(y>0)
                    		{
                   		day=d;
                   		month=m;
                   		year=y;
      		   		}
                	}
		}

	 	if(m==4 || m==6 || m==9 || m==11)
		{

			if(d>0&&d<31)
                	{
                   		if(y>0)
                    		{

                     		month=m; 
                      		day=d;
                      		year=y;
                   		}
			}
        	}
        	if(m==2)
        	{
                	if(d>0&&d<29)
                	{
                 		if(y>0)
                 		{
                  		month=m;
                  		day=d;
                    		year=y; 
                  		}
                 	}

          	}
   }
    else
     {
      month=1;
      day=1; 
      year=2000;
    }



format='D';
}


void Date::Show()                                    // shows date of object in one of the three format settings
{
int yrs;

        if(format=='D')
                cout << month <<'/'<< day << '/' << year;

        else if(format=='T')
        {
                if(month<10)
                cout<< '0' <<month<<'/';

                else
                cout<<month<<'/';

                if(day<10)
                cout<< '0' <<day<<'/';

                else
                cout<<day<<'/';

                yrs= year%100;

                if(yrs==0)
                cout <<"00";
                else
                cout<<yrs<< endl;
        }

        else if(format=='L')
        {
                switch(month)
                {
                        case 1:
                        cout<<"Jan"<<' ';
                        break;
                        case 2:
                        cout<<"Feb"<<' ';
                        break;
                        case 3:
                        cout<<"Mar"<<' ';
                        break;
                        case 4:
                        cout<<"Apr"<<' ';
                        break;
                        case 5:
                        cout<<"May"<<' ';
                        break;
                        case 6:
                        cout<<"Jun"<<' ';
                        break;
                        case 7:
                        cout<<"Jul"<<' ';
                        break;
                        case 8:
                        cout<<"Aug"<<' ';
                        break;
                        case 9:
                        cout<<"Sept"<<' ';
                        break;
                        case 10:
                        cout<<"Oct"<<' ';
                        break;
                        case 11:
                        cout<<"Nov"<<' ';
                        break;
                        case 12:
                        cout<<"Dec"<<' ';
                        break;

                }
        cout <<day<<", "<<year;
        }
}

bool Date::SetFormat(char f)
{
	if( f =='D'|| f =='L'|| f =='T')
	{
		
		format=f;
		return true;	
	}
	else
        return false;
	
}

bool Date::Set(int m, int d, int y)                         // sets date to a valid entry
{
         if(m>0&&m<13)
    {
        if(m==1 || m==3 || m==5 || m==7 || m==8 || m==10 || m==12)
         {
                if(d>0&&d<32)
                  {
                  if(y>0)
		  {
		  month=m;         
                  day=d;
		  year=y;
                  return true;
                  }
                 }
	}
	
        if(m==4 || m==6 || m==9 || m==11 )
                  {
                if(d>0&&d<31) 
                   {
                   if(y>0)
                  {
                  month=m;
                  day=d;
                  year=y;
                  return true; 
                    }
		 }  
            }
        if(m==2)
        {
                if(d>0&&d<29)
                   {
                   if(y>0)
                  {
                  month=m;
                  day=d;
                  year=y;
                  return true;
                    }
                 }
            }
  


}
        else
	return false;
	
}



void Date::Increment(int d)       // increments date to a valid date
{
int dnew,mnew,ynew;
dnew=d+day;
int dnew2;
mnew=month;
ynew=year;
	for(int i=0; i<dnew; i++)
	{

		if(mnew==1 || mnew==3 || mnew==5 || mnew==7 || mnew==8 || mnew==10 || mnew==12)
         	{
                	if(dnew>=31)
                        {
		        mnew++;
                           dnew=dnew-31;
                           if(dnew==0)
                           dnew=1;
				if(mnew>=13)
                                {
                                mnew=1;
				ynew++;
                                }
		 	}
		}

             if(mnew==4 || mnew==6 || mnew==9 || mnew==11)
                {

                        if(dnew>=30)
                        {
                        mnew++;
                            dnew=dnew-30;
                           if(dnew==0)
                           dnew=1;
                                if(mnew>=13)
                                {
                                mnew=1;
                                ynew++;
                                }
                        }
                }
                
               if(mnew==2)                                 
                {
                        if(dnew>=28)
                        {
                        mnew++;
                           dnew=dnew-28;
                           if(dnew==0)
                           dnew=1;
                               dnew=1;
                                if(mnew>=13)
                                {      
                                mnew=1;
                                ynew++;
                                }
                        }
                }

 






}

day=dnew;
month=mnew;
year=ynew;
}



int Date::GetMonth() const
{
return month;
}


int Date::GetYear() const
{
return year;
}


int Date::GetDay() const 
{
return day;
}



int Date::Compare(const Date& d)           // compares dates and returns 1 0 -1 depending if the dates come before of after each other chronologically
{
if (year<d.year)
 {
      return -1;
 }
   else if (year>d.year)
 {
      return 1;
   }
   else if (month<d.month)
 {
      return -1;
   }
   else if (month>d.month)
 {
      return 1;
   }
if(month==d.month && year==d.year && day== d.day)
return 0;
}


void Date::Input()            // asks the user to inputs a valid date and forcing them to retry if the get it wrong.
{
int x,y,z;
char w,t;
cout<<"Please enter a date: ";
cin>>x>>w>>y>>t>>z;
while(Set(x,y,z)==false)

{
cout<<"Invalid date Please try again ";
cin>>x>>w>>y>>t>>z;

}
 
cout <<"Thank You!";

}
