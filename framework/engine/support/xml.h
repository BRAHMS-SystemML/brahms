/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitchinson
	URL: http://brahms.sourceforge.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
________________________________________________________________

	Subversion Repository Information (automatically updated on commit)

	$Id:: xml.h 2258 2009-10-31 18:27:37Z benjmitch            $
	$Rev:: 2258                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-31 18:27:37 +0000 (Sat, 31 Oct 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_SUPPORT_XML
#define INCLUDED_BRAHMS_SUPPORT_XML



////////////////	NAMESPACE

namespace brahms
{

	namespace xml
	{



////////////////	XML NODE CLASS

	/*

		We implement some parts of the W3C DOM specification exactly
		and some parts inexactly. Those that are represented exactly
		are marked "W3C".

		http://www.w3.org/TR/DOM-Level-3-Core/core.html

		The W3C DOM is complex, having 12 different node types to
		represent the full gamut of XML. Here, we support only minimal
		XML, and we demote attributes and textual content from node
		status. Thus, our XMLNode's are all of the same type: "element".
		Our interface, thus, is a merging of the W3C "Node" and "Element"
		interfaces, with some inappropriate functions left out.

		In contrast to the W3C DOM, we do not represent text content
		of a tag as a separate sub-node or set of nodes. Rather, we
		use the simpler model that each tag has a text string and a
		set of child nodes that are elements.

		All but one functions that return a pointer to an object return a
		pointer to an object that still belongs to the called node,
		not a copy. The returned object, thus, should not be deleted.
		To take charge of a node, use the function removeChild(), which
		deletes the node from amongst the called node's children, and
		returns it to the caller who now owns it. The related removal
		function that deletes it rather than returning it is deleteChild().

		This attempt to be true to the W3C introduces some idiosyncrasies,
		such as the different conventions for accessing the nodeName
		(nodeName()) and the userData (getUserData()). In spite of that,
		this seems wiser than inventing a new interface.

	*/

	struct XMLAttr
	{
/*W3C*/ std::string			name;
/*W3C*/ std::string			value;
	};

	struct XMLControl
	{
		XMLControl()
		{
			readonly = false;
		}

		bool readonly;
	};

	class XMLNode;
	typedef std::vector<XMLNode*> XMLNodeList;
	typedef std::vector<XMLAttr> XMLAttrList;

	class XMLNode : public RegisteredObject
	{

	public:

		//	constructors
		XMLNode(const char* nodeName = NULL, const char* nodeText = NULL);
		XMLNode(std::istream& src);
		XMLNode(const XMLNode& e);
		XMLNode& operator=(const XMLNode& e);
		~XMLNode();

		//	W3C "Node" interface
/*W3C*/ const char*				nodeName() const;
		void					nodeName(const char* nodeName); // W3C nodeName is read-only
/*W3C*/ XMLNode*				parentNode() const;
/*W3C*/ const XMLNodeList*		childNodes() const;
/*W3C*/ XMLNode*				firstChild();
/*W3C*/ XMLNode*				lastChild();
/*W3C*/ XMLNode*				previousSibling();
/*W3C*/ XMLNode*				nextSibling();
/*W3C*/	XMLAttrList*			attributes();
/*W3C*/	XMLNode*				insertBefore(XMLNode* newChild, XMLNode* refChild = NULL); // returns newChild
/*W3C*/	XMLNode*				replaceChild(XMLNode* newChild, XMLNode* oldChild); // NOTE returns oldChild, which you are now in charge of (i.e. swaps with you, one for the other)
/*W3C*/	XMLNode*				removeChild(XMLNode* oldChild); // this takes charge of the node
		void					deleteChild(XMLNode* oldChild); // this deletes the node (not a W3C function)
/*W3C*/	XMLNode*				appendChild(XMLNode* newChild);
/*W3C*/ bool					hasChildNodes() const;
/*W3C*/ XMLNode*				cloneNode(bool deep) const;
/*W3C*/ bool					hasAttributes() const;
/*W3C*/ bool					isSameNode(const XMLNode* node) const;
		INT32					getUserData() const; // W3C has user data type "Any", so we're not really achieving that
		void					setUserData(INT32 data);

		//	W3C "Element" interface
/*W3C*/	const char*				getAttribute(const char* name) const; // throws if absent (not quite W3C semantics), but...
		const char*				getAttributeOrNull(const char* name) const; // ...this is, it's just our naming convention works better this way round
/*W3C*/	void					setAttribute(const char* name, const char* value);
/*W3C*/	void					removeAttribute(const char* name);
/*W3C*/ XMLNodeList				getElementsByTagName(const char* nodeName) const; // special value for all is NULL (W3C uses "*")
/*W3C*/ bool					hasAttribute(const char* name) const;

		//	extensions to the W3C interfaces
		bool					hasChild(const char* nodeName) const;
		XMLNode*				getChild(const char* nodeName, UINT32 index = 0);
		XMLNode*				getChildOrNull(const char* nodeName, UINT32 index = 0);
		void					removeAttributes();
		void					removeChildren();
		void					clear();

		//	implementation-specific interface
		bool					hasNodeText() const;
		const char*				nodeText() const;
		void					nodeText(const char* text);

		//	document parse/serialize interface
		void					parse(std::istream& src);
		const char*				parse(const char* text);
		void					serialize(std::ostream& dst, UINT32 indent = 0, UINT32 flags = 0, XMLNode* start = NULL, XMLNode* stop = NULL) const;



		//	content
		struct
		{
			XMLNodeList children;
			XMLAttrList attributes;
			//std::string name; /* now provided by RegisteredObject base class! */
			std::string text;
		}
		element;

		//	other data
		XMLNode* parent;			//	parent if this node has one, else NULL
		const XMLControl* control;	//	points to the control object for this node
		INT32 userData;				//	not used by the framework

		//	the control object is inherited when appendChild() is used; but to begin with, it must be set explicitly on the root node
		void setControl(const XMLControl* control);

		//	feedback
		string nodeNoun(bool recursed = false) const;
	};




////////////////	NAMESPACE

	}
}



////////////////	INCLUSION GUARD

#endif


