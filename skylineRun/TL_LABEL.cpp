//============================================================================
// Name        : query.cpp
// Author      : slhuang
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include "head2.h"
#pragma once
const int SZ_INT = sizeof(int);
const int SZ_PTR = sizeof(void*);
const long int BLK_SZ = 4194304;  //1024*1024*4 sizeof blk for I/O
const int SZ_VERTEX = sizeof(int);
const int VERTEX_PER_BLK = BLK_SZ / SZ_VERTEX;
int *scc, *dag, *topo, **TL, *tlsize;
char* read_buff_1, *read_buff_2;
char label_name[50];
int tgraph = 0, pt = 0;
FILE *sccf, *dagf, *topof, *TLf, *tlsizef;


void load(FILE *filename, int * label) {
	int * buff = (int *)read_buff_1;
	int num_read = fread(buff, SZ_VERTEX, VERTEX_PER_BLK, filename);
	int ptr_buff = 0, ptr = 0;
	while (!feof(filename) || ptr_buff != num_read) {
		label[ptr] = buff[ptr_buff];
		ptr_buff++, ptr++;
		if (ptr_buff == num_read) {
			num_read = fread(buff, SZ_VERTEX, VERTEX_PER_BLK, filename);
			ptr_buff = 0;
		}
	}
}
void load_2() {
	int * buff = (int *)read_buff_1;

	int num_read = fread(buff, SZ_VERTEX, VERTEX_PER_BLK, TLf);
	int ptr1 = 0, ptr2 = 0, ptr_buff = 0;
	while (!feof(TLf) || ptr_buff != num_read) {
		TL[ptr1][ptr2] = buff[ptr_buff];
		ptr_buff++, ptr2++;
		if (ptr2 == tlsize[ptr1])
			ptr1++, ptr2 = 0;
		if (ptr_buff == num_read) {
			num_read = fread(buff, SZ_VERTEX, VERTEX_PER_BLK, TLf);
			ptr_buff = 0;
		}
	}
}

void getLabelName(char* tmp) {
	tgraph = pt;
	for (char *i = tmp; *i != '\0'; i++) {
		label_name[tgraph++] = *i;
	}
	label_name[tgraph] = '\0';
}
// queryNode -- int from ; int to ;
vector<int> judgeReachable(char *dataFilename,char *ind_filename, vector<queryNode> query_list) {
	tgraph = 0;
	pt = 0;
	int sccN;
	FILE *infile = fopen(dataFilename, "r");
	int ret = fscanf(infile, "%d", &sccN);
	int queryNum = query_list.size();
	vector<int> res;
	res.resize(queryNum);
	int i = 1, j = 0;
	for (char *i = ind_filename; *i != '\0'; i++) {
		label_name[tgraph++] = *i;
	}
	pt = tgraph;
	char dag_l[20] = "_dag_label", topo_l[20] = "_topo_label", tlsize_l[20] = "_tlstart", TL_l[20] = "_TL";
	getLabelName(dag_l);
	dagf = fopen(label_name, "rb");   //dag file
	getLabelName(topo_l);
	topof = fopen(label_name, "rb");  //topological level file
	getLabelName(TL_l);
	TLf = fopen(label_name, "rb");  // label(TFhierarchy) file
	getLabelName(tlsize_l);
	tlsizef = fopen(label_name, "rb"); //each vertex's label size file 

	dag = (int*)malloc(SZ_INT * sccN);   //dag 
	topo = (int*)malloc(SZ_INT * sccN);  //topological level
	tlsize = (int*)malloc(SZ_INT * 2 * sccN); // each vertex's label size
	read_buff_1 = (char *)malloc(BLK_SZ);
	
	load(tlsizef, tlsize);  //load each vertex's label size 
	
	load(dagf, dag);  //load dag
	load(topof, topo); //load topological level
	
	TL = (int**)malloc(SZ_PTR * 2 * sccN);
	for (int i = 0; i < 2 * sccN; i++) {
		TL[i] = (int*)malloc(SZ_INT * tlsize[i]);
	}
	load_2();  //load label
	free(read_buff_1);

	//   query
	int* s = (int*)malloc(SZ_INT*queryNum);
	int* t = (int*)malloc(SZ_INT*queryNum);

	for (vector<queryNode>::iterator it = query_list.begin(); it != query_list.end(); it++) {
		s[j] = (*it).from;
		t[j] = (*it).to;
		j++;
	}
	/*FILE * query_file =fopen(query_filename, "r");

	int ret;
	for (int i = 0; i < queryNum; i++) {
		ret=fscanf(query_file, "%d %d\n", &s[i], &t[i]);
	}

	int reach, unreach1, unreach2, unreach3;
	reach=unreach1=unreach2=unreach3=0;
	*/
	int p, q, i2, j2, size1, size2;
	int *intArray1, *intArray2;
	j = 0;
	for (int i = 0; i < queryNum; i++, j++) {

		//	query(s, t);
		p = s[i];
		q = t[i];
		if (dag[p] != dag[q]) {    //after callapsing into super node, in the same connected DAG?
			res[j] = 0;
		}
		else {
			if (topo[p] >= topo[q]) {      //topo means the topo order
				res[j] = 0;
			}
			else {
				i2 = j2 = 0;

				//SL should int** type
				intArray1 = TL[p];
				intArray2 = TL[q + sccN];

				//SLsize[i] is the size of SL[i]
				size1 = tlsize[p];
				size2 = tlsize[q + sccN];
				while ((i2 < size1) && (j2 < size2)) {
					//SL[0~vertexN-1] store the out label of each vertex, SL[vertexN~2*vertexN-1] store the in label of each vertex

					if (intArray1[i2] < intArray2[j2]) {
						++i2;
					}
					else if (intArray1[i2] > intArray2[j2]) {
						++j2;
					}
					else {//		if (intArray1[i2] == intArray2[j2]) 
						res[j] = 1;
						break;
					}
				}

				if ((i2 == size1) || (j2 == size2)) {
					res[j] = 0;
				}
			}
		}
	}
	
	free(s);
	free(t);
	for (int i = 0; i < 2 * sccN; i++) {
		free(TL[i]);
	}
	free(TL);
	free(tlsize);
	//   free(scc);
	free(dag);
	free(topo);
	return res;
}