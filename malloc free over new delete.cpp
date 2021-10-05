//C:\Users\Vicky\Desktop\Repository\Concepts-of-CPP

#include <iostream>
#include <string>
using namespace std;

class employee
{
	private:
		
		char name[20];
		int age;
		float sal;
		
	public:
		
		employee()
		{
			cout<<endl<<"reached zero-argument contructor";
			strcpy(name, " ");
			age=0;
			sal=0.0;
		}
		
		employee(char *n, int a, float s)
		{
			cout<<endl<<"reached three-argument constructor";
			strcpy(name, n);
			age=a;
			sal=s;
		}
		
		void setdata(char *n, int a, float s)
		{
			strcpy(name, n);
			age=a;
			sal=s;
		}
		
		void showdata()
		{
			cout<<endl<<name<<"\t"<<age<<"\t"<<sal;
		}
		
		~employee()
		{
			cout<<endl<<"reached destructor"<<endl;
		}
};

int main()
{
	employee *p;
	p=new employee;
	p->setdata("sanjay", 23, 4500.50);
	
	employee *q;
	q=new employee("ajay", 24, 3400.50);
	
	p->showdata();
	q->showdata();
	
	delete p;
	delete q;
	return 0; 
}
