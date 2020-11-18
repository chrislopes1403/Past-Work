
#include <iostream>
using namespace std;

#define NODES 6
#define Iterations 5

struct node
{
	int id;
	int distance2start;
	int  previous;
};


void Init();
void print_cost();
void print_iterations();
void set_cost(int FROM, int TO, int d);
void Bellman();

struct node node_list[NODES];
int Iteration_table[Iterations][NODES];
int cost_matrix[NODES][NODES];

int main()
{


	Init();
	//----------------------------
	set_cost(0, 1, 10);
	set_cost(0, 5, 8);

	set_cost(1, 3, 2);

	set_cost(2, 1, 1);

	set_cost(3, 2, -2);

	set_cost(4, 1, -4);
	set_cost(4, 3, -1);

	set_cost(5, 4, 1);
	//----------------------------

	Bellman();
	print_cost();
	print_iterations();

	//-------------------------------
	int j = 0;
	int k;
	while (j < 6)
	{
		cout << "Enter a node from 1-5 press 6 to quit:";
		cin >> j;
		if (j > 0 && j <6)
		{
			int k;
			do
			{
				cout << j << " --> " << node_list[j].previous << endl;
				k = j;
				j = node_list[j].previous;
			} while (node_list[k].previous > 0);
		}

	}
	cout << endl << "Goodbye!" << endl;
return 0;
}


void Bellman()
{

	int table[6] = { 0,0,0,0,0,0 };
	for (int k = 0; k < 1; k++)
	{
		for (int i = 0; i <5; i++)
		{
			for (int j = 0; j < NODES; j++)
			{
				if (i > 0)
				{
					Iteration_table[i][j] = Iteration_table[i - 1][j];
					table[j] = Iteration_table[i - 1][j];
				}
			}

			for (int j = 0; j < NODES; j++)
			{
				if (cost_matrix[i][j] != 0)
				{
					if (cost_matrix[i][j] < Iteration_table[i][j])
					{
						Iteration_table[i][j] = cost_matrix[i][j] + table[i];
						node_list[j].previous = i;
					}

				}

			}

				for (int b = 0; b < NODES; b++)
				{
					if (Iteration_table[i][b] > 0 && Iteration_table[i][b] < 100)
					{
						for (int d = 0; d < NODES; d++)
						{
							if (cost_matrix[b][d] != 0)
						{
								if (Iteration_table[i][d] > Iteration_table[i][b] + cost_matrix[b][d])
								{
									Iteration_table[i][d] = Iteration_table[i][b] + cost_matrix[b][d];
									node_list[d].previous = b;
								}
							}

						}

					}

				}
				printf("\n Iteraton %d\n", i);
				print_iterations();
		}
	}



}

void Init()
{
	for (int i = 0; i < NODES; i++)
	{
		node_list[i].id = i;
		node_list[i].previous = -1;
		node_list[i].distance2start = 0;
	}

	for (int i = 0; i < NODES; i++)
	{
		for (int j = 0; j < NODES; j++)
		{
			cost_matrix[i][j] = 0;
		}

	}

	for (int i = 0; i < Iterations; i++)
	{
		for (int j = 0; j < NODES; j++)
		{
			Iteration_table[i][j] = 100;
		}
	}

	for (int i = 0; i < Iterations; i++)
	{

		Iteration_table[i][0] = 0;
	}

}

void set_cost(int FROM, int TO, int d)
{
	cost_matrix[FROM][TO] = d;
}




void print_cost()
{
	cout << endl;
	cout << " *cost matrix* " << endl;

	for (int i = 0; i < NODES; i++)
	{
		for (int j = 0; j < NODES; j++)
		{
			 cout << "["<<cost_matrix[i][j]<<"]";
		}
		cout << endl;
	}

}

void print_iterations()
{
	cout << endl;
	cout << "* Iterations Table* " << endl;
	for (int i = 0; i < Iterations; i++)
	{
		for (int j = 0; j < NODES; j++)
		{
			cout << "[" << Iteration_table[i][j] << "]";
		}
		cout << endl;
	}

}

