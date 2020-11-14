/*
CHRIS lOPES
COP4530 SUMMER 2019
HW3
*/


#ifndef LISTCLASS_H
#define LISTCLASS_H

#include <iostream>
#include <cstdio>
#include <algorithm>


template<typename Object>
class listclass
{
private:
struct Node
{
        Object data;
        Node *prev;
        Node *next;
Node( const Object & d = Object{ }, Node * p = nullptr, Node * n = nullptr )
:data{ d }, prev{ p }, next{n}{}
Node( Object && d, Node * p = nullptr, Node * n = nullptr )
:data{ std::move( d ) }, prev{ p }, next{ n}{}
friend class theiterator;
};
public:
        class theiterator
        {
        public:
        theiterator():iter_ptr{NULL}
	{}
        theiterator & operator++()
	{
        iter_ptr= iter_ptr->next;
	
	if(iter_ptr==nullptr)
        {}
	else
        return *this;
	}
	theiterator operator++ ( int )
        {
            theiterator old = *this;
            ++( *this );
            return old;
        }
	Object & operator* ( ) const
        { 
	return retrieve( ); 
	}
	bool operator!= ( const theiterator & rhs ) const
          { return !( *this == rhs ); }
        bool operator== ( const theiterator & rhs ) const
          { return iter_ptr == rhs.iter_ptr;  }
	protected:
        struct Node* iter_ptr;
      	Object & retrieve( ) const
	{return iter_ptr->data;}
	theiterator( Node *p ): iter_ptr{p} 
        { }
        friend class listclass<Object>;
        };
listclass()
{
size=0;
}
~listclass()
{
delete head;
}

int getSize() 
{
int x=size; 
return x; 
}
void back_push(Object x)
{
Node*temp= new Node;
temp->data=x;			                
	if(size==0)
	{
	head=temp;
	head->next=nullptr;
	head->prev=nullptr;	        
	}
	else if(size==1)
	{
	tail=temp;
	tail->prev=head;
	head->next=tail;
	tail->next=nullptr;
	}
	else if(size>1)
	{
		Node*tracker=head;
		for(int i=0; i<size-1; i++)
		{
		tracker=tracker->next;
		}
	tracker->next=temp;
	temp->next=nullptr;
	temp->prev=tracker;
	tail=temp;
	}
size++;
}
theiterator begin()
{
return theiterator(head);    
}
theiterator end()
{
return theiterator(tail);
}
private:
int size=0;
Node* head;
Node* tail;
friend class theiterator;
};
//#include "list-CL.cpp"
#endif



