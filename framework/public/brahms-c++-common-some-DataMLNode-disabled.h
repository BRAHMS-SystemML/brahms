
/*

I THINK THESE ARE DANGEROUS...
WE SHOULDN'T BE KEEPING REFERENCES TO XML NODES ACROSS EVENTS, SO WHY WOULD THESE BE NEEDED?

DataMLNode(const DataMLNode& src)=Create a new interface, identical to the argument $src$ (i.e. perform a shallow copy).
DataMLNode& operator[[[EQUALS]]](const DataMLNode& src)=Set the object to be an interface identical to the argument $src$ (i.e. perform a shallow copy). Note that this does not create any new XML, it just creates a second identical DataML interface to an existing XML node.

		DataMLNode(const DataMLNode& src)
		{
			//	only copying the interface, not the underlying node
			cache = src.cache;
			flags = src.flags;
			prec = src.prec;
			xmlNode = src.xmlNode;
		}

		DataMLNode& operator=(const DataMLNode& src)
		{
			if (this == &src) return *this; // handle self-assignment gracefully

			//	only copying the interface, not the underlying node
			cache = src.cache;
			flags = src.flags;
			prec = src.prec;
			xmlNode = src.xmlNode;

			return *this;
		}



DON'T KNOW WHAT THIS IS FOR

	std::string getNodeNounSub() const
	{
		std::string name;
		//if (node->parent) name = getNodeNounSub(node->parent);
		name += "<" + std::string(xmlNode->nodeName()) + ">";
		return name;
	}

DANGEROUS!

		DataMLNode()
		{
			//	this leaves the DataMLNode in an "unassigned" state (xmlNode == NULL), which
			//	should be detected by all member functions and generate an error. an unassigned
			//	node can only have one thing done to it: assign().
			xmlNode = NULL;
			cache.type = TYPE_UNSPECIFIED;
			//cache.dims = [];
			cache.numberOfRealElements = 0;
			flags = 0;
			prec = PRECISION_NOT_SET;
		}

DOES NOTHING

		~DataMLNode()
		{
			//	we're only an interface, so don't delete the underlying node
		}
*/

/*
		
DataMLNode duplicate() const=This functions performs a deep copy of the object - that is, it copies the underlying XML node as well as the interface.

DANGEROUS - why would you want to do this anyway?
		
		DataMLNode duplicate() const
		{
			//	duplicate the actual wrapped XMLNode, as well as the interface
			DataMLNode newNode = *this;
			newNode.xmlNode = new XMLNode(*newNode.xmlNode);
			return newNode;
		}
*/

/*

void setDims(Dims& dims)=Set the dimensions of the underlying array.

		void setDims(Dims& dims)
		{
			//	not implemented yet
			berr << E_NOT_IMPLEMENTED;
		}

		void setCell(UINT32 index, DataMLNode node)
		{
			DataMLNode oldNode = getCell(index);
			oldNode = node;
		}

*/
