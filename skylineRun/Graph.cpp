#include "TL_pre_file.cpp"

class Graph
{
private:
	vector<vector<int>> rNodes;	//	存储有向图
	vector<vector<int>> cNodes;	//	存储有向图相反方向的边
	// 修改图的数据结构
	map<int, int> rNodesMap;	//	行索引
	map<int, int> cNodesMap;	//	列索引
	set<int> keyList;	//	存储关键词序列
	map<int,int> conTable;	//	存储结点的强连通关系
	Graph(){}
public:
	//	单例模式，返回graph对象
	static Graph& getInstance(){
        static Graph instance;
        return instance;
    }
	
	//	返回关键词序列
	vector<int> getKeyList(){
		vector<int> res(this->keyList.begin(),this->keyList.end());
		return res;
	}

	//	返回关键词序列长度
	int getKeyListSize(){
		return this->keyList.size();
	}

	//	判断两点是否在同一个强连通分量中
	bool stronglyConnect(int v,int w){
		return this->conTable[v]==this->conTable[w];
	}

	//	插入from结点对应的一组边,如果type值为keyword，则将tos数组存入this->keyList关键词序列中
	void insertEdge(int from,vector<string> tos,string type){
		int mapIndex;
		//	将tos的string类型转化为int
		vector<int> intTos;

		if(type=="keyword"){	//	插入关键词
			for(vector<string>::iterator it=tos.begin();it!=tos.end()-1;it++){
				intTos.push_back(stoi(*it));
				this->keyList.insert(stoi(*it));
			}
		}else{	//	插入链接边
			for(vector<string>::iterator it=tos.begin();it!=tos.end()-1;it++){
				intTos.push_back(stoi(*it));
			}
		}
		
		//	插入正向边索引
		if(this->rNodesMap.find(from)==this->rNodesMap.end()){	//	结点from未在索引中
			mapIndex=this->rNodes.size();
			this->rNodesMap[from]=mapIndex;
			this->rNodes.push_back(vector<int>());
		}else{	//	结点from已在索引中
			mapIndex=this->rNodesMap[from];
		}
		
		//	插入正向边
		this->rNodes[mapIndex].insert(this->rNodes[mapIndex].end(),intTos.begin(),intTos.end());	//	将tos插入对应的this->rNodes中

		for(vector<int>::iterator it=intTos.begin();it!=intTos.end();it++){
			//	插入反向索引
			if(this->cNodesMap.find(*it)==this->cNodesMap.end()){	//	结点to未在索引中
				mapIndex=this->cNodes.size();
				this->cNodesMap[*it]=mapIndex;
				this->cNodes.push_back(vector<int>());
			}else{	//	结点to已在索引中
				mapIndex=this->cNodesMap[*it];
			}
			//	插入反向边
			this->cNodes[mapIndex].push_back(from);
		}
	}

	// 将关键字整合到图中，成为图中节点，便于计算距离
	void readGraph(string edgeFileName, string keywordFileName,string conTableFileName) {
		// 打开文件
		vector<string> res;
		ifstream infile;
		string s;

		// 读取edge文件
		infile.open(edgeFileName.data());   // 将文件流对象与文件连接起来 
		assert(infile.is_open());   // 若失败,则输出错误消息,并终止程序运行
		getline(infile, s);	// 跳过第一行
		while (getline(infile, s)) // 读取所有数据
		{
			vector<string> temp = Util::split(s, ": ");
			int from = stoi(temp.front());	// 边的起始点
			vector<string> tos = Util::split(temp.back(), ",");	// 边的终点集合
			this->insertEdge(from,tos,"node");
		}
		infile.close();

		// 读取keyword文件
		infile.open(keywordFileName.data());   // 将文件流对象与文件连接起来 
		assert(infile.is_open());   // 若失败,则输出错误消息,并终止程序运行
		getline(infile, s);
		while (getline(infile, s)) // 读取所有数据
		{
			vector<string> temp = Util::split(s, ": ");
			int from=stoi(temp.front());
			vector<string> tos = Util::split(temp.back(), ",");	// 获取关键词
			this->insertEdge(from,tos,"keyword");
		}
		infile.close();

		//	读取强连通分量关系
		infile.open(conTableFileName.data());   // 将文件流对象与文件连接起来 
		assert(infile.is_open());   // 若失败,则输出错误消息,并终止程序运行
		while (getline(infile, s)) // 读取所有数据
		{
			vector<string> temp = Util::split(s, ",");
			this->conTable[stoi(temp.front())]=stoi(temp.back());
		}
		infile.close();
	}

	// 将图结构写入文件中，格式为：节点 出度数 连接的顶点1,连接的顶点2,.....
	/*
	void write(string outputFileName) {
		ofstream outfile;
		outfile.open(outputFileName.data());   // 将文件流对象与文件连接起来 
		assert(outfile.is_open());   // 若失败,则输出错误消息,并终止程序运行

		for(map<int,int>::iterator it=this->rNodesMap.begin();it!=this->rNodesMap.end();it++){
			outfile<<(*it).first<<" "<<this->rNodes[(*it).second].size();
			for(vector<int>::iterator it_inner=this->rNodes[(*it).second].begin();it_inner!=this->rNodes[(*it).second].end();it_inner++){
				outfile<<" "<<*it_inner;
			}
			if(it!=this->rNodesMap.end()){
				outfile<<endl;
			}
		}
		outfile.close();
	}
	*/

	// 判断点p是否在队列中
	//	############################################可优化#############################################
	bool contains(queue<int> qu,int p){
		queue<int> tmp(qu);
		while(!tmp.empty()){
			if(p == tmp.front()){
				return true;
			}
			tmp.pop();
		}
		return false;
	}
	
	// 计算图中两结点的最短路径,采用bidirational Search算法
	int minDist(int from,int to) {
		map<int,bool> visitf;	//	判断结点是否已经遍历过
		map<int,bool> visitb;
		queue<int> quf,qub;
		int distf = 0 ,distb = 0 , index = 0, p = 0, nextFf = from, nextFb=to;
		quf.push(from);
		qub.push(to);
		visitf[from]=true;
		visitb[to]=true;

		while (!quf.empty() && !qub.empty()) {
			if(!quf.empty()){
				p = quf.front();
				quf.pop();
				if(p == to || contains(qub,p)){
					return distf+distb;
				}
				// 获取点p的邻接结点
				if(this->rNodesMap.find(p)!=this->rNodesMap.end()){	//	该结点有邻接节点
					for(vector<int>::iterator it=this->rNodes[this->rNodesMap[p]].begin();it!=this->rNodes[this->rNodesMap[p]].end();it++){
						if(visitf.find(*it)==visitf.end()){	//	结点p未访问
							visitf[*it]=true;
							quf.push(*it);
						}
					}
				}
				// 若点p是当层最后一个结点，路径距离+1
				if(nextFf==p){
					distf++;
					nextFf=quf.back();
				}
			}
			
		

			if(!qub.empty()){
				p = qub.front();
				qub.pop();
				if(p == from || contains(quf,p)){
					return distf+distb;
				}
				// 获取点p的邻接结点
				if(this->cNodesMap.find(p)!=this->cNodesMap.end()){	//	该结点有邻接节点
					for(vector<int>::iterator it=this->cNodes[this->cNodesMap[p]].begin();it!=this->cNodes[this->cNodesMap[p]].end();it++){
						if(visitb.find(*it)==visitb.end()){	//	结点p未访问
							visitb[*it]=true;
							qub.push(*it);
						}
					}
				}
				// 若点p是当层最后一个结点，路径距离+1
				if(nextFb==p){
					distb++;
					nextFb=qub.back();
				}
			}
		}
		return -1;	// 不可达
	}
};