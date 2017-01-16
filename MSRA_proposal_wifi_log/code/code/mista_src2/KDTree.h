// Library taken from Asmodehn (http://asmodehn.home.dyndns.org/)
// and very slightly modified by myself

#ifndef KDTREE_HH
#define KDTREE_HH 1

#define DIMENSIONS 3


//int dimensions = DIMENSIONS;
extern int dimensions;


#include <vector> 
#include <iostream>
using namespace std;

//Classe Template pour les objets a classer dans le KDTree
//Cette classe doit disposer d'un code d'erreur, pour indiquer une recherche sans resultat par exemple

//Classe pour les objets a classer dans le KDTree
template <class Object> class KDObject
{
	//Meme nombre de dimensions pour tous les objets, attention a la coherence
//	static int dimensions = DIMENSIONS;
	
	public:
	//Tableau de coordonnées, Attention a la dimensions lors des acces aux coords
	float* coords;
	//Stockage de l'objet
	const Object obj;
	
	KDObject(const Object& o=Object::ERROR) : obj(o) {coords=new float[dimensions];}
	KDObject(const Object& o,float* c) : obj(o) {coords=c;}
	KDObject(const KDObject& toStore) : obj(toStore.obj)
	{
		coords=new float[dimensions];
		for(int i=0;i<dimensions;i++)
		{
			coords[i]=toStore.coords[i];
		}
	}
	~KDObject() {delete coords;}
	
	//Accesseurs aux coordonnees
	inline float operator [] (int i) const { return *(coords + i % dimensions); }
	inline float &operator [] (int i) { return *(coords + i % dimensions); }
	
	//Calcul des distances
	inline float dist(const float* point) const
	{
		float maxim=fabs(point[0]-coords[0]);
		float tmp;
		for (int i=1;i<dimensions;i++)
			if((tmp=fabs(point[i]-coords[i]))>maxim)
				maxim=tmp;
		return maxim;
	}
	
};

//Type retourné lors d'une requete de recherche au KDTree
template <class Object> class KDObjDist
{
	public:
	float dist;
	Object object; //Pour recopier la valeur et laisser le KDTree intact

	
	KDObjDist(const Object & o=Object::ERROR, float d=-1.0f) : dist(d) , object(o) {}
	KDObjDist(const KDObject<Object> & o, float d=-1.0f) : dist(d), object(o->obj) {}
};

template <class Object> class KDTree
{
//	static int dimensions=DIMENSIONS;
			
	class KDNode
	{
		private:
		const KDObject<Object> _data;
		public:
		KDNode* left;
		KDNode* right;
#ifndef REC
		KDNode* parent;
#endif
		//Constructeur et Destructeur
		KDNode(const KDObject<Object>& d=KDObject<Object>(Object::ERROR)) : _data(d) 
		{
			left=NULL;right=NULL;
#ifndef	REC
			parent=NULL;
#endif
		}
		~KDNode() {delete left; delete right;}
		//Accesseur
		const KDObject<Object>& data(void) const { return _data;}
	};
	
	KDNode* root;
	
	//Fonctions de manipulation internes
#ifdef REC
	bool insert(KDNode*& start,short dimstart, KDNode* node);
#else
	bool insert(KDNode* start,short dimstart, KDNode* node);
#endif
	//suppr(KDNode* start,int dimstart);
	bool minmax(KDNode* start,short dimstart,float* min,float* max);
	bool findNN(KDNode* start,short dimstart,const float* point,const KDNode*& neighbor, float& dist);
	bool findNear(KDNode* start,short dimstart,const float* point, float radius,vector<const KDNode*>& neighbor, vector<float>& dist);
	//findinAABox(KDNode* start,short dimstart,const float*& min,const float*& max);
#ifndef REC
		KDNode* pivot(const KDNode* start, short dimstart);
#endif
#ifdef REC
	bool balance(KDNode*& start,short dimstart);
#else
	bool balance(KDNode* start,short dimstart);
#endif
	long count(KDNode* start);
		
	public:
			
	//Constructeurs et Destructeurs
	KDTree() {root=NULL;}
	~KDTree() {delete root;}
	
	//Fonctions de manipulation globales
	bool insert(const KDObject<Object>& data);
	//suppr(const KDObject* data);
	bool minmax(float* min,float* max);
	KDObjDist<Object> findNN(const float* point);
	vector< KDObjDist<Object> > findNear(const float* point,const float radius);
	//findinAABox(const float*& min,const float*& max);
	bool balance(void);
	long count(void);
	void stats(void);
	
};

//Because of the template class, implementation must be here :(
//#pragma implementation // seems to be unneeded on my OS, it works fine without it.

template <class Object> bool KDTree<Object>::insert(KDNode* start, short dimstart, KDNode* node)
{
	//On detache le noeud a inserer si il etait attache
	node->parent=NULL;
	
	//On parcours l'arbre a inserer
	KDNode* tnode=node;
	bool goup=false;//pour signaler de remonter
	do
	{
		if ( tnode->left!=NULL && !goup )
		{
			node=tnode;tnode=tnode->left;goup=false;
		}
		else if ( (tnode->right!=NULL) && (!goup || node==tnode->left) )
		{
			node=tnode;tnode=tnode->right;goup=false;
		}
		else //Dans les autres cas on remonte
		{
			node=tnode;tnode=tnode->parent;goup=true;
		}
		
		if(goup)//Quand on remonte
		{
			//On insere l'ancien noeud dans l'arbre
			short dim=dimstart;
			unsigned char curson=' ';
			KDNode* prev=start;
			KDNode* temp=prev;
			while (temp!=NULL)
			{
				prev=temp;
				if (node->data()[dim]<=prev->data()[dim])
				{
					temp=temp->left;curson='l';
				}
				else
				{
					temp=temp->right;curson='r';
				}
				dim=(dim+1) % dimensions;
			}
			
			if (temp!=root)
			{
				switch(curson)
				{
					case 'l' : prev->left=node;break;
					case 'r' : prev->right=node;break;
					default : cerr << "ERROR in Insert" << endl; return false;
				}
				node->parent=prev;
			}
			else// prev==temp==null==root => en root
				root=node;
			//On detache le noeud de son ancien arbre
			node->left=NULL;
			node->right=NULL;
		}
	}while (tnode!=NULL);
	if(!goup) {cerr << "ERROR in Insert Loop" << endl; exit(-1);}
	//On est sorti de la boucle en remontant l'arbre
	
	return true;
}
template <class Object>
bool
KDTree<Object>::minmax(KDNode* start, short dimstart, float* min,float* max)
{
	short dim=dimstart;
	//On detache le noeud de depart
	KDNode* pmem=start->parent;
	start->parent=NULL;
	
	//ALGO A OPTIMISER
	KDNode* temp=start;
	bool goup=false;//pour signaler de remonter
	while(temp!=NULL)
	{
		if(!goup)
		{
			//On fait les test min / max
			for (short i=0;i<dimensions;i++)
			{
				if (start->data()[i]<min[i]) min[i]=start->data()[i];
				if (start->data()[i]>max[i]) max[i]=start->data()[i];
			}
		}
		
		if ( temp->left!=NULL && !goup )
		{
			start=temp;temp=temp->left;goup=false;dim=(dim+1)%dimensions;
		}
		else if ( (temp->right!=NULL) && (!goup || start==temp->left) )
		{
			start=temp;temp=temp->right;goup=false;dim=(dim+1)%dimensions;
		}
		else //Dans les autres cas on remonte
		{
			start=temp;temp=temp->parent;goup=true;dim=(dim-1)%dimensions;
		}

	}
	if(!goup) {cerr << "ERROR in MinMax" << endl; exit(-1);}
	//On est sorti de la boucle en remontant l'arbre
	//On rattache le noeud de depart
	start->parent=pmem;
	return true;
}

template <class Object>
bool
KDTree<Object>::findNN(KDNode* start,short dimstart,const float* point,const KDNode*& neighbor, float& dist)
{
	int dim=dimstart;
	//On detache le noeud de depart
	KDNode* pmem=start->parent;
	start->parent=NULL;
	
	KDNode* temp=start;
	bool goup=false;//pour signaler de remonter
	while(temp!=NULL)
	{
		if(!goup)
		{
			//On fait les test de distance
			float sum=start->data().dist(point);
			if (sum <dist || neighbor==NULL) 
			{
				dist=sum;
				neighbor=start;
			}
		}
					
		if ( temp->left!=NULL && !goup && (dist>=0 && point[dimstart]-dist<= temp->data()[dimstart] ))
		{
			start=temp;temp=temp->left;goup=false;dim=(dim+1)%dimensions;
		}
		else if ( (temp->right!=NULL) && (!goup || start==temp->left)  && (dist>=0 && point[dimstart]+dist> start->data()[dimstart] ))
		{
			start=temp;temp=temp->right;goup=false;dim=(dim+1)%dimensions;
		}
		else //Dans les autres cas on remonte
		{
			start=temp;temp=temp->parent;goup=true;dim=(dim-1)%dimensions;
		}
	}
	if(!goup) {cerr << "ERROR in findNN" << endl; exit(-1);}
	//On est sorti de la boucle en remontant l'arbre
	//On rattache le noeud de depart
	start->parent=pmem;
	return true;
}

template <class Object>
bool
KDTree<Object>::findNear(KDNode* start,short dimstart,const float* point, const float radius,vector<const KDNode*>& neighbor, vector<float>& dist)
{
	short dim=dimstart;
	//On detache le noeud de depart
	KDNode* pmem=start->parent;
	start->parent=NULL;
	
	KDNode* temp=start;
	bool goup=false;//pour signaler de remonter
	while(temp!=NULL)
	{
		if (!goup)
		{
			//On fait les test de distance
			float sum=temp->data().dist(point);
			if (sum <=radius) 
			{
				dist.push_back(sum);
				neighbor.push_back(temp);
			}
		}

		//On choisit le prochain noeud a tester
		if ( (temp->left!=NULL) && (!goup) && (point[dim]-radius<=temp->data()[dim]))
		{
			start=temp;temp=temp->left;goup=false;dim=(dim+1)%dimensions;
		}
		else if ( (temp->right!=NULL) && (!goup || start==temp->left) && (point[dim]+radius>temp->data()[dim]) )
		{
			start=temp;temp=temp->right;goup=false;dim=(dim+1)%dimensions;
		}
		else //Dans les autres cas on remonte
		{
			start=temp;temp=temp->parent;goup=true;dim=(dim+dimensions-1)%dimensions;
		}
	}
	if(!goup) {cerr << "ERROR in findNear" << endl; exit(-1);}
	//On est sorti de la boucle en remontant l'arbre
	//On rattache le noeud de depart
	start->parent=pmem;
	
	return true;
}

//Recupere le pivot dans un sous arbre
//Pivot = +proche du milieu de l'hyperrectangle du sous arbre
template <class Object>
typename KDTree<Object>::KDNode* KDTree<Object>::pivot(const KDNode* start, short dimstart)
{
	float* min=new float[dimensions];
	float* max=new float[dimensions];
	//On cherche les dimensions de l'hyperrectangle du sous arbre
	const KDNode* piv=start;
	
	minmax(const_cast<KDNode*>(start),dimstart,min,max);
	//Calcul du centre de dimstart
	float* mid=new float[dimensions];
	for (short i=0;i<dimensions;i++) mid[i]=(min[i]+max[i]) / 2.0f;
	
	//On cherche le plus proche voisin
	float dist;
	findNN(const_cast<KDNode*>(start),dimstart,mid,piv,dist);
	
	return const_cast<KDNode*>(piv);
}

template <class Object>
bool
KDTree<Object>::balance(KDNode* start, short dimstart)
{
	short dim=dimstart;
	//On detache le noeud de depart
	KDNode* pmem=start->parent;
	start->parent=NULL;
	
	KDNode* temp=start;
	bool goup=false;//pour signaler de remonter
	while(temp!=NULL)
	{
		if(!goup)
		{
			//On recupere le pivot
			KDNode* piv=pivot(temp,dim);
			
			if (piv!=temp)
			{
				//On sauvegarde les sous-arbres de piv
				KDNode* memleft=piv->left;
				KDNode* memright=piv->right;
			
				//On detache piv
				if(piv->parent!=NULL)
				{
					if (piv->parent->left==piv) piv->parent->left=NULL;
					else if (piv->parent->right==piv) piv->parent->right=NULL;
				}
			
				//piv prend la place du noeud courant
				piv->left=NULL;
				piv->right=NULL;
				piv->parent=temp->parent;
				if (piv->parent!=NULL)
				{
					if (piv->parent->left==temp) piv->parent->left=piv;
					else if (piv->parent->right==temp) piv->parent->right=piv;
				}
							
				//On reinsere le noeud temp dans l'arbre, ainsi que ses descendants
				//Il sera detache de son pere courant pour etre placé ailleurs
				insert(root,0,temp);
				
				//le pivot devient le noeud courant
				temp=piv;
				
				//On insere les sous-arbres memorisés 
				if (memleft!=NULL) insert(root,0,memleft);
				if (memright!=NULL) insert(root,0,memright);
			}
		}	
		
		if ( temp->left!=NULL && !goup )
		{
			start=temp;temp=temp->left;goup=false;dim=(dim+1)%dimensions;
		}
		else if ( (temp->right!=NULL) && (!goup || start==temp->left) )
		{
			start=temp;temp=temp->right;goup=false;dim=(dim+1)%dimensions;
		}
		else //Dans les autres cas on remonte
		{
			start=temp;temp=temp->parent;goup=true;dim=(dim-1)%dimensions;
		}
		
	}
	if(!goup) {cout << "ERROR" << endl; exit(-1);}
	//On est sorti de la boucle en remontant l'arbre
	//On rattache le noeud de depart
	start->parent=pmem;
	return true;
}

template <class Object>
long
KDTree<Object>::count(KDNode* start)
{
	long nbNodes=0;
	//On detache le noeud de depart
	KDNode* pmem=start->parent;
	start->parent=NULL;
	
	KDNode* temp=start;
	bool goup=false;//pour signaler de remonter
	while(temp!=NULL)
	{
		if (!goup)
		{
			nbNodes++;
		}
		
		if ( temp->left!=NULL && !goup )
		{
			start=temp;temp=temp->left;goup=false;
		}
		else if ( (temp->right!=NULL) && (!goup || start==temp->left) )
		{
			start=temp;temp=temp->right;goup=false;
		}
		else //Dans les autres cas on remonte
		{
			start=temp;temp=temp->parent;goup=true;
		}
				
		
		/*** ALGO OK***
		//Mais pas pratique
		while(!goup)
		{
			start=temp;
			
			nbNodes++;
			
			if(start->left!=NULL) {temp=start->left;}
			else if(start->right!=NULL) {temp=start->right;}
			else {temp=start->parent;goup=true;}
		}
		while(goup && temp!=NULL)
		{
			if(start==temp->left && temp->right!=NULL)
			{
				start=temp;
				temp=temp->right;
				goup=false;
			}
			else
			{
				start=temp;
				temp=temp->parent;
			}
		}*/
	}
	if(!goup) {cout << "ERROR" << endl; exit(-1);}
	
	//On est sorti de la boucle en remontant l'arbre
	//On rattache le noeud de depart
	start->parent=pmem;
	return nbNodes;
}


//Fonctions publiques
template <class Object>
bool KDTree<Object>::insert(const KDObject<Object>& data)
{
	KDNode* newone=new KDNode(data);
	return insert(root,0,newone);
}
template <class Object>
bool KDTree<Object>::minmax(float* min,float* max)
{
	if (root!=NULL)
	{
		for (short i=0; i<dimensions ; i++)
		{
			min[i]=root->data()[i];
			max[i]=root->data()[i];
		}
		return minmax(root,0,min,max);
	}
	else
		return false;
}

template <class Object>
KDObjDist<Object> KDTree<Object>::findNN(const float* point)
{
	KDObjDist<Object> res;
	const KDNode* neighb=NULL;
	float dist(res.dist);

	if (root!=NULL)
	{
		bool found=findNN(root,0,point,neighb,dist);
		if(found)
		{
			res=neighb;
			res.dist=dist;
		}
	}
	return res;
	
}
template <class Object>
vector< KDObjDist<Object> > KDTree<Object>::findNear(const float* point, float radius)
{
	vector<const KDNode*> neighb;
	vector< KDObjDist<Object> > neighbor;
	vector<float> dist;

	if (root!=NULL)
	{
		bool res=findNear(root,0,point,radius,neighb,dist);
		if (res) 
			for (unsigned int i=0;i<neighb.size();i++)
			{
				neighbor.push_back(KDObjDist<Object>(neighb[i]->data().obj,dist[i]));
			}
	}
	return neighbor;
}
template <class Object>
bool KDTree<Object>::balance(void)
{
	if (root!=NULL)
		return balance(root,0);
	else
		return false;
}
template <class Object>
long KDTree<Object>::count(void)
{
	if (root!=NULL)
		return count(root);
	else return 0;
}
template <class Object>
void KDTree<Object>::stats(void)
{
	long nbNodes=count();
	cout << "NbNodes Stored : " << nbNodes << endl;
	if (nbNodes>0)
	{
		cout << "AABoundingBox : " << endl;
		float* min=new float[dimensions];
		float* max=new float[dimensions];
		minmax(min,max);
		for (int i=0;i<dimensions;i++)
			cout << "Dim " << i << " : Min = " <<min[i] <<" , Max = "<<max[i] <<endl;
		
		cout << "Memory Used : " << nbNodes *  sizeof(KDNode) / 1024 << endl;
	}
	else
	{
		cout << "No Nodes... No tests made..." << endl;
	}
}

#endif /* !KDTREE_HH */




