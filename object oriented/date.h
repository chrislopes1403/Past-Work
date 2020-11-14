class Date

{
public:

Date();
Date(int m, int d, int y);
int m;
int d;
int y;
char f;

void Input();
void Show();
void Increment(int d=1);
//void Increment();
bool Set(int m, int d, int y);
bool SetFormat(char f='D');
int GetMonth() const; 
int GetYear() const; 
int GetDay() const; 
int Compare(const Date& d);




private:
int month;
int day;
int year;
char format; 
};
