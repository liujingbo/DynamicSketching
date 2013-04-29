
#ifndef HIERARCHYGRAPH_201208291143
#define HIERARCHYGRAPH_201208291143

//#include<stdio.h>
#include<stdlib.h>
#include <list>
#include <set>
#include <vector>
//#include<string.h>
#define maxVertices 30
#define maxEdges 100

using std::list;

class HierarchyGraph{
public:
	typedef struct Edge
	{
		int from,to,weight;
	}Edge;

	int graph[maxVertices][maxVertices];
	/* Input graph must be undirected,weighted and connected*/
	Edge E[maxEdges];
	int parent[maxVertices];// tree
	list<Edge> treeEdges;

	vector<int> order;

	static int hierarchy_compare(const void * x,const void * y)
	{
		return (*(Edge *)x).weight - (*(Edge *)y).weight;
	}
	void init(int vertices)
	{
		int iter=0;
		for(iter=0;iter<vertices;iter++)
		{
			parent[iter]=-1;
		}
	}
	int Find(int vertex)
	{
		if(parent[vertex]==-1)return vertex;
		return parent[vertex] = Find(parent[vertex]); /* Finding its parent as well as updating the parent 
		of all vertices along this path */
	}
	void Union(int parent1,int parent2)
	{
		/* This can be implemented in many other ways. This is one of them */
		parent[parent1] = parent2;
	}
	void Kruskal(int vertices,int edges)
	{
		memset(graph,-1,sizeof(graph)); /* -1 represents the absence of edge between them */
		/* Sort the edges according to the weight */
		qsort(E,edges,sizeof(Edge), hierarchy_compare);
		/* Initialize parents of all vertices to be -1.*/
		init(vertices);
		treeEdges.resize(0);
		int totalEdges = 0,edgePos=0,from,to,weight;
		Edge now;
		while(totalEdges < vertices -1)
		{
			if(edgePos==edges)
			{
				/* Input Graph is not connected*/
				exit(0);
			}
			now = E[edgePos++];
			from = now.from;
			to = now.to;
			weight=now.weight;
			/* See If vetices from,to are connected. If they are connected do not add this edge. */
			int parent1 = Find(from);
			int parent2 = Find(to);
			if(parent1!=parent2)
			{
				graph[from][to] = weight;
				treeEdges.push_back(now);
				Union(parent1,parent2);
				totalEdges++;
			}
		}
	}

	bool find(vector<int> q, int t){

		for(int i=0;i<q.size();i++){
			if(q[i] == t) return true;
		}
		return false;
	}
	HierarchyGraph(int v_, int e_, vector<vector<int>> E_)
	{
		int vertices,edges;
		//scanf("%d%d",&vertices,&edges);
		vertices = v_;
		edges =e_;
		int iter,jter;
		int from,to,weight;
		for(iter=0;iter<edges;iter++)
		{
			//scanf("%d%d%d",&from,&to,&weight);
			from = E_[iter][0];
			to = E_[iter][1];
			weight = E_[iter][2];

			E[iter].from = from; 
			E[iter].to = to; 
			E[iter].weight = weight;
		}
		/* Finding MST */
		Kruskal(vertices,edges);

		/* Printing the MST */
		/*for(iter=0;iter<vertices;iter++)
		{
			for(jter=0;jter<vertices;jter++)
		{
			if(graph[iter][jter]!=-1)
		{
			printf("Vertex %d and %d, weight %d\n",iter,jter,graph[iter][jter]);
		}
		}
		}*/

			// setting the drawing order
		// temporarily start from patch #0
		order.clear();
		RecursiveTraverse(0);

		return;
	}

	void RecursiveTraverse(int root){
		order.push_back(root);
		for(list<Edge>::iterator it = treeEdges.begin(); it!=treeEdges.end();it++){
			if(it->from == root && !find(order,it->to )){
				RecursiveTraverse(it->to);
			}
			else if (it->to==root && !find(order, it->from)){
				RecursiveTraverse(it->from);
			}
		}
		return;
	}


};



#endif