

/*Chris Lopes*/

#include <stdio.h>
#include <stdlib.h>     //* malloc, free, rand */
#include <limits.h> /* For INT_MAX */ 

#define infinite 100

struct node
{
	int id;
	struct  node * back;
	int distance2start;
	int  previous;

};

void print_node(void);
void Add_Node_From(int id);
void Set_Edge(int FROM, int TO, int d);
void Set2d(int id);
void Dijkstra();
void bubbleSort(int arr[],int n);
void swap(int *xp, int *yp);
int print2Smallest(int arr[], int arr_size);
int smallest;


int node_count = 0;

struct node node_list[5];
int edge[5][5];

int main(void)
{
	printf("running main \n");
	for (int i = 0; i < 5; i++)
	{
		node_list[i].id = i;
		node_list[i].distance2start = 0;
		node_list[i].previous = 0;
	}

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
			edge[i][j] = 0;
	}

	//--------------------------
	Set_Edge(0, 1, 6);
	Set_Edge(0, 3, 1);

	Set_Edge(1, 3, 2);
	Set_Edge(1, 2, 5);
	Set_Edge(1, 4, 2);

	Set_Edge(3, 4, 1);
	Set_Edge(4, 2, 5);
	//--------------------------
	print_node();
	Dijkstra();

	int menu = 0;

	while (menu != 5) 
	{
		printf("\nselect a 1 - 4: to see shortest path form start" );
		printf("\nselect 5 to quit: ");
		scanf("%d",&menu);

		int i = menu;
		if (menu <= 4 && menu >= 1)
		{
			while (node_list[i].distance2start > 0)
			{
				printf("node[%d] ---> node[%d]\n", node_list[i].id, node_list[i].previous);
				i = node_list[i].previous;
			}
		}
	}

	

	
	return 0;
}
void Dijkstra()
{
	int table[5] = { 0,infinite,infinite,infinite,infinite };
	int table_temp[5];

	int visited[5] = { 0,0,0,0,0 };
	int visited2[5] = { 1,0,0,0,0 };
	int  previous;
	int x;
	int i = 0;
	int j = 0;
	int smallest_index;
	int o = 0;
	while (visited2[0] == 0|| visited2[1] == 0 || 
		visited2[2] == 0 || visited2[3] == 0 || visited2[4] == 0 )
	{
	

			for (int j = 0; j < 5; j++)
			{
				if (edge[i][j] > 0)
				{
					x = node_list[i].distance2start + edge[i][j];
					
					if (x < table[j]) 
					{
						table[j] = x;
						visited[j] = j;
						node_list[j].previous = i;
						node_list[j].distance2start = x;
					}

				}

			}
			
			for (int i = 0; i < 5; i++)
			{
				table_temp[i] = table[i];
			}
			int n = sizeof(table) / sizeof(table[0]);	
			 bubbleSort(table_temp,n);
	
			 int count=j;
			 
			 int count2 = 0;
			 for (int j = 0; j < 5; j++)
			 {
				 if (table_temp[j] >0) 
				 {
				 count2++;
					 if (count == count2)
					 { 
						 smallest = table_temp[j];
						 break; 
					 }
				 }
			 }

			int u=0;
			for (int j = 0; j < 5; j++)
			{	
				if (table[j] == smallest  && visited[j]>0)
				{
					u = visited[j]; 
					break;
				}
			}
		
			j++;

			visited2[u] = 1;
			i = u;
			o++;
	}

			
	/*

			//int n = sizeof(table) / sizeof(table[0]);
			// bubbleSort(table,n);

		for (int j = 0; j < 5; j++)
		{
			printf("[%d] ", table_temp[j]);
		}

		printf("\n");

		for (int j = 0; j < 5; j++)
		{
			printf("[%d] ", table[j]);

		}

		printf("\n");

		for (int j = 0; j < 5; j++)
		{
			printf("[%d] ", visited[j]);

		}

		printf("\n");

		for (int j = 0; j < 5; j++)
		{
			printf("[%d] ", visited2[j]);

		}

		*/

}

void bubbleSort(int arr[],int n)
{

	int i, j;
	for (i = 0; i < n - 1; i++)

		// Last i elements are already in place    
		for (j = 0; j < n - i - 1; j++)
			if (arr[j] > arr[j + 1])
				swap(&arr[j], &arr[j + 1]);
}

void swap(int *xp, int *yp)
{
	int temp = *xp;
	*xp = *yp;
	*yp = temp;
}

void Set_Edge(int FROM, int TO,int d)
{
	edge[FROM][TO] = d;
	edge[TO][FROM] = d;
}


void print_node(void)
{
	printf("\n *cost matrix*\n");
	printf("\n *nodes A-E are represented as 0-4*\n");
	printf("\n *row are nodes and colums of that row are the of the nodes and the distances*\n");
	printf("\n *example 0,1 is distance form node 0 to node 1*\n");
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			printf("[%d]",edge[i][j]);
		}
		printf("\n");
	}


}


 
 

